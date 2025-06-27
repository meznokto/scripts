#!/usr/bin/python3

#
# imunifylogs.py - a utility to query imunify360-agent
# and check imunify360 logs.
#
# Author: Ryan Henry <rhenry@a2hosting.com>
#

import argparse
import sys
import ipaddress
import subprocess
import json
import datetime
import re
from shlex import split

class bcolors:
	HEADER = '\033[95m'
	GREEN = '\033[92m'
	YELLOW = '\033[93m'
	RED = '\033[91m'
	FAIL = '\033[91m'
	ENDC = '\033[0m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'

# location of the imunify360 console log    
log = "/var/log/imunify360/console.log"

parser=argparse.ArgumentParser(
    description="A tool to query Imunify360"
)

parser.add_argument("-i", "--ip", help="IP address to search for", required=False)
parser.add_argument("-d", "--domain", help="domain name to search for", required=False)
parser.add_argument("-p", "--period", help="number of days to search (default is 7)", default="7", required=False)
args=parser.parse_args()

# append 'd' to the period so we check that many days
period = args.period + "d"

# make sure we have either an IP or a domain to search for
if args.ip:
	# we have an IP address, make sure it's valid
	try:
		myip = ipaddress.ip_address(args.ip)
	except:
		print("Invalid IP address")
		sys.exit()
	cmd = 'imunify360-agent get --period ' + period + ' --by-abuser-ip ' + args.ip + ' --json'
	# use shlex.split() to escape shell characters
	cmd = split(cmd)
	# check that we have the correct number of arguments
	if len(cmd) != 7:
		print("Invalid number of arguments received!")
		parser.print_help()
		sys.exit()
elif args.domain:
	cmd = 'imunify360-agent get --period ' + period + ' --search ' + args.domain + ' --json'
	# use shlex.split() to escape shell characters
	cmd = split(cmd)
	# check that we have the correct number of arguments
	if len(cmd) != 7:
		print("Invalid number of arguments received!")
		parser.print_help()
		sys.exit()
else:
	parser.print_help()
	sys.exit()

if args.ip:
	# check if the IP is on any lists
	print("Checking Imunify360 lists...")
	blockcmd = 'imunify360-agent ip-list local list --by-ip ' + args.ip + ' --json'
	# use shlex.split() to escape shell characters
	blockcmd = split(blockcmd)
	try:
		blocks = subprocess.run(blockcmd, stdout=subprocess.PIPE, shell=False)
	
		try:
			blocks = json.loads(blocks.stdout)
			if blocks['max_count'] != 0:
				if blocks['counts']['server']['white'] != 0:
					print(bcolors.GREEN + "IP is whitelisted on server." + bcolors.ENDC)
				if blocks['counts']['server']['drop'] != 0:
					print(bcolors.RED + "IP is in server drop list." + bcolors.ENDC)
				if blocks['counts']['server']['captcha'] != 0:
					print(bcolors.RED + "IP is in server captcha list." + bcolors.ENDC)
				if blocks['counts']['server']['splashscreen'] != 0:
					print(bcolors.RED + "IP is in server splashscreen list." + bcolors.ENDC)
				if blocks['counts']['cloud']['white'] != 0:
					print(bcolors.RED + "IP is in cloud whitelist." + bcolors.ENDC)
				if blocks['counts']['cloud']['drop'] != 0:
					print(bcolors.RED + "IP is in cloud drop list." + bcolors.ENDC)
				if blocks['counts']['cloud']['captcha'] != 0:
					print(bcolors.RED + "IP is in cloud captcha list." + bcolors.ENDC)
				if blocks['counts']['cloud']['splashscreen'] != 0:
					print(bcolors.RED + "IP is in cloud splashscreen list." + bcolors.ENDC)
			
				for item in blocks['items']:
					date = datetime.datetime.fromtimestamp(item['ctime']).strftime('%Y-%m-%d %H:%M:%S')
					if item['purpose'] == "white":
						print(bcolors.GREEN + "White List: ", end="")
					elif item['purpose'] == "captcha":
						print(bcolors.YELLOW + "CAPTCHA List: ", end="")
					elif item['purpose'] == "drop":
						print(bcolors.RED + "DROP List: ", end="")
					elif item['purpose'] == "spashscreen":
						print(bcolors.YELLOW + "Splashscreen: ", end="")
					else:
						print(bcolors.YELLOW, end="")
					print(date + "\t" + item['comment'] + bcolors.ENDC)

				print("")
			else:
				print(bcolors.GREEN + "IP not found on any list" + bcolors.ENDC + "\n")
		except:
			print("Unable to parse JSON: " + myjson)
	except Exception as e:
		print("Error: " + str(e))
		
	# check log file to see if it has any info
	print("Console log entries (if available):")
	captchacmd = 'grep ' + args.ip + ' ' + log
	# use shlex.split() to escape shell characters
	captchacmd = split(captchacmd)
	try:
		with subprocess.Popen(captchacmd, stdout=subprocess.PIPE, shell=False) as proc:
			for line in proc.stdout:
				if b'CaptchaEvent' in line:
					textline = line.decode('utf-8')
					# get the "CaptchaEvent....processed in" part of the log line:
					res = re.search(r'CaptchaEvent\((.*?)\)\Wprocessed\ in', textline)
					# remove the "CaptchaEvent(" and ") processed in" parts of the line:
					myjson = re.sub(r'\)\ processed\ in', '', re.sub(r'CaptchaEvent\(', '', res.group()))
					# remove existing double-quotes
					myjson = myjson.replace('"', '')
					# replace single quotes with doulbe-quotes to make JSON
					myjson = myjson.replace("'", '"')
					# a few special cases also need fixing
					myjson = myjson.replace("True", '"True"')
					myjson = myjson.replace("False", '"False"')
					myjson = myjson.replace('IPv4Network("', '"IPv4Network(')
					myjson = myjson.replace('")', ')"')
					# load the JSON
					try:
						result = json.loads(myjson)
						date = datetime.datetime.fromtimestamp(result['timestamp']).strftime('%Y-%m-%d %H:%M:%S')
						if result['event'] == "PASSED":
							# customer completed CAPTCHA
							print(bcolors.GREEN + date + " CAPTCHA passed" + bcolors.ENDC)
						else:
							parts = result['message'].split(" ")
							print(bcolors.YELLOW + date + " CAPTCHA " + parts[10] + parts[6] + bcolors.ENDC)
					except:
						print("Unable to parse JSON: " + myjson)
				elif b'SensorIncident' in line:
					textline = line.decode('utf-8')
					# extract the "SensorIncident...processed in" part of the log line
					res = re.search(r'SensorIncident\((.*?)\)\Wprocessed\ in', textline)
					if res:
						# this is a SensorIncident. Remove the "SensorIncident(" and ") processed in" parts:
						myjson = re.sub(r'\)\ processed\ in', '', re.sub(r'SensorIncident\(', '', res.group()))
						# fix quotes around bracketed output
						myjson = myjson.replace("['", "'[")
						myjson = myjson.replace("']", "]'")
						# remove existing double-quotes
						myjson = myjson.replace('"', '')
						# replace single quotes with double-quotes
						myjson = myjson.replace("'", '"')
						# fix special cases:
						myjson = myjson.replace("True", '"True"')
						myjson = myjson.replace("False", '"False"')
						myjson = myjson.replace('service_gen", "noshow', 'service_gen, noshow')
						myjson = myjson.replace('service_im360", "noshow', 'service_im360, noshow')
						myjson = myjson.replace('service_i360", "noshow', 'service_i360, noshow')
						# load the JSON
						try:
							result = json.loads(myjson)
							date = datetime.datetime.fromtimestamp(result['timestamp']).strftime('%Y-%m-%d %H:%M:%S')
							if result['plugin_id'] == 'modsec':
								if result['status_code'] == '200':
									# modsec was triggered, but didn't block the request, so make the output green
									print(bcolors.GREEN, end="")
								elif result['status_code'] == '403':
									# modsec was triggered and a 403 error returned,
									# make the output red
									print(bcolors.RED, end="")
								else:
									# modsec was triggered and there was a response other than 200 or 403. 
									# May or may not have been blocked, so make the output yellow 
									print(bcolors.YELLOW, end="")
								print(date + " " + result['plugin_id'] + " " + result['rule'] + " " + result['name'] + "\n\tResponse: " + result['status_code'] + " Request: " + result['domain'] + result['advanced']['uri'] + bcolors.ENDC)
							elif result['plugin_id'] == 'ossec':
								if result['rule'] == 31130:
									print(bcolors.GREEN + date + " " + result['name'] + bcolors.ENDC)
								else:
									print(bcolors.YELLOW + date + " " + result['plugin_id'] + " " + str(result['rule']) + " " + result['message'] + bcolors.ENDC)
						except:
							# output our bad JSON if it's not about a successful api token usage
							if not re.search('Successful operation using API token', myjson):
								print("Unable to parse JSON: " + myjson)
								
	except Exception as e:
		print("Error: " + str(e))
	print("")

# check for incidents returned by imunify360-agent
print("Checking Imunify360 history...")
try:
	results = subprocess.run(cmd, stdout=subprocess.PIPE, shell=False)
	try:
		results = json.loads(results.stdout)

		# output the results
		if results['items']:
			print(bcolors.HEADER + "Date/Time\t\tIP\t\tRule\t\tDomain\t\t\tDescription" + bcolors.ENDC)
			for result in results['items']:
				date = datetime.datetime.fromtimestamp(result['timestamp']).strftime('%Y-%m-%d %H:%M:%S')

				if result['domain']:
					# information is about a domain
					print(bcolors.YELLOW + date + "\t" + result['abuser'] + "\t" + result['plugin'] + " " + result['rule'] + "\t" + result['domain'] + "\t" + result['name'] + bcolors.ENDC)
				elif result['plugin'] == "enhanced_dos":
					# temporarily blocked for high connections
					print(bcolors.RED + date + "\t" + result['description'] + bcolors.ENDC)
				else:
					# information is about an IP address
					print(bcolors.YELLOW + date + "\t" + result['abuser'] + "\t" + result['plugin'] + " " + result['rule'] + "\t" + "No Domain" + "\t" + result['name'] + bcolors.ENDC)
		else:
			print(bcolors.GREEN + "No results found." + bcolors.ENDC)
	except:
		print("Unable to parse JSON: " + results.stdout)
except Exception as e:
	print("Error: " + str(e))

