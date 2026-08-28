#!/bin/bash

sudo groupadd docker > /dev/null 2>&1

sudo gpasswd -a $USER docker && newgrp docker

