#!/bin/bash
# install_deps.sh — Installe les dépendances de build pour abls-satellite-libs (Fedora/RHEL)
set -e
sudo dnf install -y cmake gcc pkg-config glib2-devel abls-libs-devel