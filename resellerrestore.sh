#!/bin/bash

if [[ $# != 1 ]]; then
  echo "Usage: $0 reseller"
  echo "Restores all sub-accounts under reseller account."
  echo "It does NOT restore the reseller account!"
  echo "Reseller account must already exist and/or have been restored first."
  exit;
fi

# don't restore accounts owned by root!
if [[ $1 == root ]]; then
  echo "This script cannot be used to restore accounts owned by root!"
  exit;
fi

# Verify reseller exists
OUTPUT=$(whmapi1 --output=jsonpretty resellerstats user="$1" | jq -r '.data.reseller.user')
# OUTPUT will have the reseller's username if they exist and are a reseller
if [[ $OUTPUT == $1 ]]; then
  echo "Account $1 exists and is a reseller, continuing..."
  echo ""
else
  echo "Account $1 doesn't exist or isn't a reseller!"
  echo "Reseller cPanel account needs to be restored before using this script."
  exit;
fi

# get list of orphan accounts owned by reseller, but not the reseller themselves:
ACCOUNTS=$(jetbackup5api -F listAccounts -O json -D "orphan=1&limit=0" | jq -r '.data.accounts[] | [.owner, .username, ._id] | join(" ")' | grep "^$1 " | grep -v "^$1 $1\ " | cut -f2,3 -d' ')

# check that there are any accounts to restore
TOTAL=$(echo -n "$ACCOUNTS" | grep -c '^')

if [[ $TOTAL == 0 ]]; then
  echo "No orphan accounts found to restore!"
  exit;
fi

# variable to store backups to be restored
TORESTORE=""

# loop through $ACCOUNTS to find backups
while IFS= read -r line
do
  account=$(echo $line | cut -f1 -d' ')
  
  # check if we've already processed this user
  DONE=$(echo $TORESTORE | grep -c "$account ")
  
  if [[ $DONE != 1 ]]; then
    echo "Checking user $account..."
    
    TIMES=$(echo "$ACCOUNTS" | grep -c "$account ")
    
    if [[ $TIMES != 1 ]]; then
      # account username appears multiple times.
      # check each user ID for backups and select the most recent
      echo "Found multiple JetBackup accounts with name $account"
      echo "Finding most recent backup for $account..."
    
      # get the list of UIDs for this username
      ACCTS=$(jetbackup5api -F listAccounts -O json -D "orphan=1&limit=0" | jq -r '.data.accounts[] | [.owner, .username, ._id] | join(" ")' | grep "$1 $account" | cut -f3 -d' ')
      BACKUPS=""
    
      # fine the most recent backup for each jetbackup account
      while IFS= read -r bline
      do
        #echo -en "Account ID: $bline\t"
        
        # get info for the most recent backup of the account:
        OUTPUT=$(jetbackup5api -F listBackups -O json -D "type=1&contains=511&account_id=$bline" | jq -r '.data.backups[] | [.parent_id, .created] | join(" ")' | tail -n1)
        PID=$(echo "$OUTPUT" | cut -f1 -d' ')
        BKDATE=$(echo "$OUTPUT" | cut -f2 -d' ')
        if [[ ${#BKDATE} -ge 1 ]]; then
          #echo -en "Backup ID: $PID\t"
          #echo -en "Date: $BKDATE\n"
          EPOCH=$(date -d $BKDATE +%s)
          #echo "Epoch: $EPOCH"
          BACKUPS="$BACKUPS$account $PID $EPOCH"$'\n'
        else
          #echo -en "No backups found.\n"
          :
        fi
      done < <(printf '%s\n' "$ACCTS")
      
      # remove the trailing newline from BACKUPS
      BACKUPS=${BACKUPS%$'\n'}
      
      OURBACKUP=$(echo "$BACKUPS" | sort -k3 -n -r | head -n1 | cut -f2 -d' ')
      OURDATE=$(echo "$BACKUPS" | sort -k3 -n -r | head -n1 | cut -f3 -d' ' | date +"%Y-%m-%d")
      echo "Selected backup $OURBACKUP $OURDATE"
      TORESTORE="$TORESTORE$account $OURBACKUP $OURDATE"$'\n'
      echo ""
    else
      # username only appears once so just find the most recent backup
      uid=$(echo "$line" | cut -f2 -d' ')
  
      # get info for the most recent backup of the account:
      OUTPUT=$(jetbackup5api -F listBackups -O json -D "type=1&contains=511&account_id=$uid" | jq -r '.data.backups[] | [.parent_id, .created] | join(" ")' | tail -n1)

      parent_id=$(echo "$OUTPUT" | cut -f1 -d' ')
      backup_date=$(echo "$OUTPUT" | cut -f2 -d' ' | cut -f1 -d'T')
      echo "Selected backup $parent_id $backup_date"
      TORESTORE="$TORESTORE$account $parent_id $backup_date"$'\n'
      echo ""
    fi
  else
    # we've already processed this user, nothing to do.
    :
  fi
done < <(printf '%s\n' "$ACCOUNTS")

echo "Backups to restore:"
echo "$TORESTORE"

REALTOTAL=$(echo -n "$TORESTORE" | grep -c '^')

read -r -p "Are you sure you want to restore all $REALTOTAL of $1's orphan accounts? [y/N] " response
if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]
then
  # remove trailing newline
  TORESTORE=${TORESTORE%$'\n'} 
  
  # loop through $TORESTORE and queue the backup jobs
  while IFS= read -r line
  do
    account=$(echo $line | cut -f1 -d' ')
    backup_id=$(echo $line | cut -f2 -d' ')
    backup_date=$(echo $line | cut -f3 -d' ')
  
    echo -en "User: $account\t"

    # queue the restore job:
    echo -en "Backup date: $backup_date\t"
    RESULT=$(jetbackup5api -F addQueueItems -O json -D "type=2&snapshot_id=$backup_id" | jq -r '.message')
    echo "$RESULT"

  done < <(printf '%s\n' "$TORESTORE")
else
  echo "Cancelling operation."
fi

