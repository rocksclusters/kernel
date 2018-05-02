#!/usr/bin/python
#
# This script tries much harder to get a kickstart file. Needed for
# larger clusters.
# Magic boot parameters for node to use this method
# tracker.trackers=<ip of frontend>
# inst.ks=http://localhost/fetchRocksKS.py
# rocks.ks=http://<ip of frontend>/install/sbin/kickstart.cgi
#
# Logic of these parameters
# tracker.trackers -- this will properly allow lighttpd to start on the node
# inst.ks -- tells anaconda to do http, but direct through this cgi script
# rocks.ks -- tells this script where to fetch the real kickstart file
#
# In case of failure:
# 	slow frontend, no frontend. - create a "rescue kickstart file"
#       can ssh to the node with passwd "rescueME" 
#       attempts to leave a failure note on the frontend
#
DEFAULTURL = "https://10.1.117.1/install/sbin/kickstart.cgi"
RETRIES = 4
MINTIMEOUT = 2
MAXTIMEOUT = 20
import os
import sys
import urllib,urllib2
import random
import time
import ssl

print "# Opening /proc/cmdline looking for rocks.ks and rocks.ks.timeout"
arg = []
url = None
RESCUE_KICKSTART="""
# There was an error retrieving the kickstart file
# This is rescue kickstart that simply reboots the node to try again
#
lang en_US.UTF-8
keyboard us
install
reboot
timezone --utc America/Los_Angeles
sshpw --username root rescueME
# This presection reboots the node after 2 minutes
%%pre
if [ -f /tracker/peer-done ]; then /tracker/peer-done; fi
/bin/curl -k --max-time 5 %s
/sbin/shutdown -r +2 
%%end
"""
## read /proc/cmdline and look for rocks.ks=
try:
	f = open("/proc/cmdline")
	lines = f.readlines()
	f.close()
	arg = filter(lambda x: x[0]=="rocks.ks",map(lambda y: y.split('=')," ".join(lines).split()))
	ksTimeout = filter(lambda x: x[0]=="rocks.ks.timeout",map(lambda y: y.split('=')," ".join(lines).split()))
except Exception as e:
	print "# had exception %s" % e.__str__()

## Set the url, if misformatted use the default 
url = DEFAULTURL
try:
	if len(arg) > 0:
		url = arg[0][1]
except:
	pass	

# Set the maximum timeout, override if rocks.ks.timeout specified 
maxTimeout = MAXTIMEOUT
try:
	if len(ksTimeout) > 0:
		maxTimeout = ksTimeout[0][1]
except:
	pass

lines = sys.stdin.readlines()
query=""
macargs=""
sslContext = ssl._create_unverified_context()

try:
	request = urllib2.Request(url,query)

	print "# Looking for provisioning mac"

	for key in os.environ.keys():
		if key.startswith("HTTP_X_RHN_PROVISIONING_MAC"):

			iface = int(key.split("_")[-1])
			macHeader = "X-RHN-Provisioning-MAC-%d" % iface
			request.add_header(macHeader,os.environ[key])
			pstr = "%s: %s\r\n"%(macHeader,os.environ[key])
			macargs += pstr
	cpus = open("/proc/cpuinfo")
	lines = cpus.readlines()
	cpus.close()
	np=filter(lambda x: x.startswith('processor'),lines)	
	if len(np) > 0:
		request.add_header('np',len(np))
	# print "XXX" + str(np) + str(len(np))

except Exception as e:
	print "# urllib2 call had exception %s" % str(e) 

retries = RETRIES
## Print some debug statements
print "# URL is %s" % url
print "# Query is %s" % query
print "# Environment is %s" % str(os.environ)
print "# Macargs is %s" % macargs.replace("\r","").replace("\n"," ")

goodResponse = False
while retries > 0:
	try:
		retries = retries - 1
		response = urllib2.urlopen(request,macargs,maxTimeout,context=sslContext)
		goodResponse=True
		break

	except urllib2.HTTPError as e:
		timeout = None
		if e.headers.has_key("Retry-After"):
			timeout = int(e.headers["Retry-After"]) 
		if timeout is None:
			timeout = maxTimeout
		sleeptime = random.randint(MINTIMEOUT,timeout)
		# syslog.syslog(syslog.LOG_INFO,"ROCKS: Kickstart Service Busy. Retries left (%d) in %d secs" % (retries, sleeptime))
		time.sleep(sleeptime)
	except urllib2.URLError as e:
		# This is a generic error (bad url, ...)
		# retry until we can't anylonger
		pass
	except socket.timeout as e:
		# Timed out waiting
		pass
	except Exception as e:
		# Unknown exception, bad stuff, so just print the rescue
		# kickstart file
		break	

## Read the response
try:
	ksContents = response.read()
except:
	ksContents = ""

if goodResponse and len(ksContents) > 0:
	print ksContents 
else:
	# Try to leave a rescue note on the front end
	requestFailed = urllib2.Request(url+".FAILED")
	try:
		responseFailed = urllib2.urlopen(requestFailed,None,MAXTIMEOUT)
	except:
		# Should be a 404 (Not Found) error
		pass
	# Print the rescue kickstart file
	print RESCUE_KICKSTART % (url + ".FAILED")
	
