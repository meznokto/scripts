#!/usr/bin/python3

#
# mailbandwidth.py - a utility to collect bandwidth
# usage from dovecot logs.
#
# Usage: mailbandwidth <domain or cPanel username>
# Options:
# -m --month Month to check, 1=January, 2=February, etc. Defaults to current month
#
# Author: Ryan Henry <rhenry@a2hosting.com>
#

import argparse
import sys
import os
import subprocess
import json
import calendar
import re
import glob
import math
import logging
from shlex import split
from datetime import datetime
  
logpath = "/var/log/maillog*"

# set level=logging.DEBUG to see debugging output
logging.basicConfig(level=logging.INFO)

# display a progress bar
def progressBar(value, endvalue, bar_length=20):
        percent = float(value) / endvalue
        arrow = '-' * int(round(percent * bar_length)-1) + '>'
        spaces = ' ' * (bar_length - len(arrow))
        sys.stdout.write("\rProgress: [{0}] {1}%".format(arrow + spaces, int(round(percent * 100))))
        sys.stdout.flush()

# convert bytes into human-readable formats        
def convert_size(size_bytes):
   if size_bytes == 0:
       return "0B"
   size_name = ("B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB")
   i = int(math.floor(math.log(size_bytes, 1024)))
   p = math.pow(1024, i)
   s = round(size_bytes / p, 2)
   return "%s %s" % (s, size_name[i])
   
# used to sort dictionaries by value
def value_getter(item):
  return item[1]

# set up our command line arguments and parse them
parser=argparse.ArgumentParser(
    description="A tool to collect bandwidth usage from dovecot logs."
)

parser.add_argument('tocheck', help="Domain name or cPanel username")
parser.add_argument('-m', '--month', help="Month to check, 1=January, 2=February, etc. Defaults to current month", required=False)
args=parser.parse_args()

domains = []

# check if it's a valid domain name
pattern = re.compile(
   r'^(([a-zA-Z]{1})|([a-zA-Z]{1}[a-zA-Z]{1})|'
   r'([a-zA-Z]{1}[0-9]{1})|([0-9]{1}[a-zA-Z]{1})|'
   r'([a-zA-Z0-9][-_.a-zA-Z0-9]{0,61}[a-zA-Z0-9]))\.'
   r'([a-zA-Z]{2,13}|[a-zA-Z0-9-]{2,30}.[a-zA-Z]{2,3})$'
)
  
if pattern.match(args.tocheck):
  # It is a domain name.
  logging.debug("Domain name: " + args.tocheck)
    
  # verify domain exists on the server
  try:
    # get info on all domains
    result = subprocess.check_output(['whmapi1', '--output=jsonpretty', 'get_domain_info'], shell=False)
    alldomains = json.loads(result)
    for domain in alldomains['data']['domains']:
      if args.tocheck == domain['domain']:
        domains.append(args.tocheck)
    
  except subprocess.CalledProcessError as e:
    print("whmapi1 call failed with return code " + e.returncode + "!")
  except Exception as e:
    print("Unexpected error checking domain!")
    print("Error: " + e)
    sys.exit()

  # verify we have a domain:
  if len(domains) < 1:
    print("Domain " + args.tocheck + " not found!")
    sys.exit()
else:
  # Not a domain, so assuming it's a cPanel username.
  # Gather all their email accounts and get the domains they use.
  logging.debug("Gathering email addresses...")
  cmd = 'whmapi1 --output=jsonpretty list_pops_for user=' + args.tocheck
  cmd = split(cmd)
  try:
    output = subprocess.run(cmd, stdout=subprocess.PIPE, shell=False)
    try:
      output = json.loads(output.stdout)
    except Exception as e:
      print("Unable to parse JSON: " + output.stdout)
      sys.exit()
    # check the metadata to make sure the request was successful
    if output['metadata']['result'] == 1:
      # success, process the email addresses to find their domains
      for address in output['data']['pops']:
        # check if domain exists in domains[] and
        # add it if it doesn't. (Maybe we should just get a list of 
        # the user's addons and their primary domain instead...)
        domain = address.split('@')[1]
        if not domain in domains:
          domains.append(domain)
    else:
      print("Error retrieving email information!")
      print("Is " + args.tocheck + " a valid cPanel account?")
      sys.exit()
      
  except Exception as e:
    print("Error: " + str(e))

logging.debug("Domain Names:")
logging.debug(domains)

# if a month was given on the command line,
# convert it to an abbeviation like used
# in the dovecot logs and the full name
# for display purposes
month = ""
longmonth = ""
if args.month:
  try:
    month = calendar.month_abbr[int(args.month)]
    longmonth = calendar.month_name[int(args.month)]
  except Exception as e:
    print("Invalid month provided. Please provide a number:")
    print("January = 1")
    print("February = 2")
    print("March = 3, etc")
    print(str(e))
    sys.exit()
else:
  # No month provided, use the current month.
  month = calendar.month_abbr[int(datetime.now().month)]
  longmonth = calendar.month_name[int(datetime.now().month)]

print("Checking logs for " + longmonth + "\n")

# collect the log entries from the mail logs
logs = []
for filename in glob.glob(logpath):
  print("Processing log " + filename + "...")
    
  # if the filename has a date appended, check that 
  # it's not before the month we're interested in
  # TOFIX: this won't work correctly around the start of a new year
  try:
    res = re.search(r'\d{8}$', filename)
    filemonth = res.group()[4:6]
    logging.debug("Month file ends: " + filemonth)
    
    if (int(filemonth) < list(calendar.month_abbr).index(month)):
      print("Skipping file due to it ending before " + longmonth)
      continue
  except Exception as e:
    # no date on filename, go ahead an process it
    logging.debug("No date on filename")
  
  with open(filename, 'r', encoding='latin-1') as file:   
    # if the log entries are for a month after the
    # one we're interested in, skip this file
    # TOFIX: this won't work correctly around the start of a new year
    first_line = file.readline()
    first_month = first_line.partition(' ')[0]
    first_month_int = list(calendar.month_abbr).index(first_month)
    
    if (first_month_int > list(calendar.month_abbr).index(month)):
      print("Skipping file due to it starting after " + longmonth)
    else:
      # this file starts before or during the month 
      # we want, so rewind to the start and process it.
      file.seek(0)
      
      # get the filesize so we can show a progress bar
      filesize = os.path.getsize(filename)
      progress=0
      
      for i in file:
        # update progress bar
        progress=progress+len(i)
        progressP=(float(progress))/filesize
        progressBar(int(progressP*100),100)

        # if this line contains one of the domain names we're looking for,
        # add it to the logs variable.
        for domain in domains:
          # We search for "@domain.com)" 
          # to avoid collecting other domains, like otherdomain.com, 
          # domain.com.uk, etc.
          if re.search("@" + domain + "\)", i):
            # also check if the log entry is for the
            # month we're interested in. Testing showed searching the domain
            # first was faster than searching the month first.
            if re.match(month, i):
              # add this line to our logs
              logs.append(i)
      print("")
    # we're done with this file, close it
    file.close()

logging.debug("Log entries found: " + str(len(logs)) + "\n")

if len(logs) == 0:
  print("No log entries found!")
  sys.exit()

imapbytesin = {}
imapbytesout = {}
pop3bytesin = {}
pop3bytesout = {}

for line in logs:
  # for imap we can look for 
  # "dovecot: imap * in=(read from client), out=(sent to client)" 
  # to get the bytes read and bytes sent to the client.
  res = re.search(r'dovecot: imap\(([\w.+-]+@[\w-]+\.[\w.-]+)\).*?bytes=(\d+)/(\d+)', line)
  if res:
    if res.group(1) not in imapbytesin:
      # we haven't seen this email address yet,
      # so create initial dictionary entries
      imapbytesin.update({res.group(1): 0})
      imapbytesout.update({res.group(1): 0})
    # update the stats for this email account
    imapbytesin.update({res.group(1): imapbytesin[res.group(1)] + int(res.group(2))})
    imapbytesout.update({res.group(1): imapbytesout[res.group(1)] + int(res.group(3))})   
  else:
    # for pop3 we look for this pattern:
    # "dovecot: pop3 * bytes=(read from client)/(sent to client)"
    res = re.search(r'dovecot: pop3\(([\w.+-]+@[\w-]+\.[\w.-]+)\).*?bytes=(\d+)/(\d+)', line)
    if res:
      if res.group(1) not in pop3bytesin:
        # we haven't seen this email address yet,
        # so create initial dictionary entries
        pop3bytesin.update({res.group(1): 0})
        pop3bytesout.update({res.group(1): 0})
      # update the stats for this email account
      pop3bytesin.update({res.group(1): pop3bytesin[res.group(1)] + int(res.group(2))})
      pop3bytesout.update({res.group(1): pop3bytesout[res.group(1)] + int(res.group(3))})
      
# Output our results
print("\nIMAP bytes uploaded:")
for email, bytes in sorted(imapbytesin.items(), key=value_getter, reverse=True):
  print(email + " " + str(convert_size(bytes)))
  
print("\nIMAP bytes downloaded:")
for email, bytes in sorted(imapbytesout.items(), key=value_getter, reverse=True):
  print(email + " " + str(convert_size(bytes)))

print("\nPOP3 bytes uploaded:")
for email, bytes in sorted(pop3bytesin.items(), key=value_getter, reverse=True):
  print(email + " " + str(convert_size(bytes)))
  
print("\nPOP3 bytes downloaded:")
for email, bytes in sorted(pop3bytesout.items(), key=value_getter, reverse=True):
  print(email + " " + str(convert_size(bytes)))

