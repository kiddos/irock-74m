#!/usr/bin/env sh

cmake -B build .
make -C build -j8

sudo cp build/irock-74m-server /usr/local/bin
sudo cp build/irock-bot /usr/local/bin

SERVER_SERVICE_NAME="irock74m-server"
SERVER_SERVICE_FILE="/etc/systemd/system/$SERVER_SERVICE_NAME.service"

echo "Creating $SERVER_SERVICE_NAME.service file at $SERVER_SERVICE_FILE..."
sudo bash -c "cat > $SERVER_SERVICE_FILE" <<EOF
[Unit]
Description=irock74m keyboard API
After=systemd-udev-settle.service local-fs.target
Wants=systemd-udev-settle.service

[Service]
User=root
ExecStart=/usr/local/bin/irock-74m-server
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

BOT_SERVICE_NAME="irock-bot"
BOT_SERVICE_FILE="/etc/systemd/system/$BOT_SERVICE_NAME.service"

echo "Creating $BOT_SERVICE_NAME.service file at $BOT_SERVICE_FILE..."
sudo bash -c "cat > $BOT_SERVICE_FILE" <<EOF
[Unit]
Description=irock74m bot
Requires=$SERVER_SERVICE_NAME.service
After=$SERVER_SERVICE_NAME.service

[Service]
User=root
ExecStart=/usr/local/bin/irock-bot
Restart=always
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

echo "Reloading systemd daemon..."
sudo systemctl daemon-reload

echo "Enabling service to start on boot..."
sudo systemctl enable $SERVER_SERVICE_NAME
sudo systemctl enable $BOT_SERVICE_NAME

echo "Starting service now..."
sudo systemctl start $SERVER_SERVICE_NAME
sudo systemctl start $BOT_SERVICE_NAME
