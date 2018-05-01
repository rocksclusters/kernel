NAME		= anaconda-rocks
PKGROOT		= /usr/share/lorax
RELEASE		= 1
RPM.FILES	= "$(PKGROOT)/*"
RPM.EXTRAS="%define _python_bytecompile_errors_terminate_build 0"
