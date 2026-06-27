#!/bin/sh

SOCLE=`grep "^ID=" /etc/os-release | cut -f 2 -d '='`

if [ "$(whoami)" != "root" ]
 then
	 echo "Only user root can run this script (or sudo)."
	 exit 1
fi

groupadd abls

if [ "$SOCLE" = "fedora" ]
 then
	echo "Configuring ABLS-RPMS repository"
	curl -fsSL https://rpms.abls-habitat.fr/abls-rpms.repo -o /etc/yum.repos.d/abls-rpms.repo

	echo "Installing RPM-based dependencies"
	dnf install -y abls-libs-devel cmake gcc pkg-config glib2-devel abls-libs-devel
fi