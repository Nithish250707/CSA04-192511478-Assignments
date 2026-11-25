#!/usr/bin/env bash
# B1.3 commands for an Ubuntu lab machine. NOT executed in this repo (host is macOS).
# Run with sudo on Ubuntu 22.04/24.04 and save the output to outputs/.
set -euxo pipefail
sudo useradd --system --create-home --shell /usr/sbin/nologin portal || true
sudo mkdir -p /opt/result-portal
sudo cp ../../Task_B2/app/app.py ../../Task_B2/app/requirements.txt /opt/result-portal/
sudo python3 -m venv /opt/result-portal/.venv
sudo /opt/result-portal/.venv/bin/pip install -r /opt/result-portal/requirements.txt
sudo chown -R portal:portal /opt/result-portal

sudo cp result-portal.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable --now result-portal
systemctl status result-portal --no-pager

sudo apt-get install -y nginx
sudo cp nginx-result-portal.conf /etc/nginx/sites-available/result-portal
sudo ln -sf /etc/nginx/sites-available/result-portal /etc/nginx/sites-enabled/result-portal
sudo rm -f /etc/nginx/sites-enabled/default
sudo nginx -t && sudo systemctl reload nginx
curl -i http://localhost/health

# Firewall: allow only SSH, HTTP and HTTPS
sudo ufw default deny incoming
sudo ufw default allow outgoing
sudo ufw allow OpenSSH
sudo ufw allow 80/tcp
sudo ufw allow 443/tcp
sudo ufw --force enable
sudo ufw status verbose
