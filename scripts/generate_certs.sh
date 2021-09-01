#!/bin/bash

mkdir ../certs 2> /dev/null
cd ../certs && openssl req -nodes -x509 -newkey rsa:4096 -sha256 -keyout private_key.pem -out certificate.pem