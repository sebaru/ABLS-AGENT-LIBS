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
	echo "Configuring ABLS-PKGS repository"
	curl -fsSL https://pkgs.abls-habitat.fr/abls-rpms.repo -o /etc/yum.repos.d/abls-rpms.repo

	echo "Installing RPM-based dependencies"
	dnf install -y abls-libs-devel cmake gcc pkg-config glib2-devel abls-libs-devel
fi

if [ "$SOCLE" = "debian" ] || [ "$SOCLE" = "raspbian" ] || [ "$SOCLE" = "ubuntu" ]
 then
	echo "Configuring ABLS APT repository"
	curl -fsSL https://pkgs.abls-habitat.fr/rpms/keys/RPM-GPG-KEY-ABLS | gpg --dearmor -o /usr/share/keyrings/abls-archive-keyring.gpg
	curl -fsSL https://pkgs.abls-habitat.fr/abls-deb.sources -o /etc/apt/sources.list.d/abls-pkgs.sources

	apt update -y
	apt install -y abls-libs-dev cmake gcc pkg-config libglib2.0-dev
fi