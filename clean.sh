#!/bin/bash

echo "Cleaning up processes"
pkill -f "manager"
pkill -f "storage"
sleep 2