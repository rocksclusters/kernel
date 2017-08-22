#Emit tracker configuration
#Interprets
#	tracker=[on|off|none]
#              - won't emit configuration is "off" of "none" is given
#                default: on
#       tracker.trackers=<IP>[;<IP>] 
#		IP address(es) of hosts that run tracker-server
#       
#       tracker.pkgservers=<IP>[;<IP>] 
#		IP address of host(s) of last resort if file cannot
#               be downloaded  
#	usual usage:
#		cat /proc/cmdline | awk -f tracker.ak > /tmp/rocks.conf
#
BEGIN {
	enable = 1;
	trackers = "";
	pkgservers = "";
}
END {
	if (length(pkgservers) == 0)
		pkgservers = trackers;
	if (length(trackers) == 0)
		trackers=pkgservers;	
	if (enable && length(trackers) > 0)
	{
		gsub(";"," ",trackers)
		gsub(";"," ",pkgservers)
		printf "var.trackers = \"%s\"\n", trackers
		printf "var.pkgservers = \"%s\"\n", pkgservers 
	}
}

/tracker=[[:alpha:]]/ {
	split($0,comps,"tracker=");
	split(comps[2],vals);
	if (match(vals[1],"off") > 0 || match(vals[1],"none") > 0)
		enable = 0;
}
/tracker.trackers=[[:alnum:]]/ {
	split($0,comps,"tracker.trackers=");
	split(comps[2],vals);
	trackers=vals[1];
}
/tracker.pkgservers=[[:alnum:]]/ {
	split($0,comps,"tracker.pkgservers=");
	split(comps[2],vals);
	pkgservers=vals[1];
}
	
		
