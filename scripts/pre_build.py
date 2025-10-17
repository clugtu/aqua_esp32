#!/usr/bin/env python3
"""
Pre-build script for ESP32 Aqua Project
Ensures config.json exists before building
"""

import os
import shutil

# PlatformIO provides env, get project directory from it
Import("env")
project_dir = env.get("PROJECT_DIR")
data_dir = os.path.join(project_dir, "data")
config_file = os.path.join(data_dir, "config.json")
config_example = os.path.join(data_dir, "config.example.json")

# Private config file location (outside of git repository)
private_config_dir = os.path.join(os.path.dirname(project_dir), "private_configs")
private_config_file = os.path.join(private_config_dir, "aqua_config.json")

def check_config():
    print("ESP32 Aqua Project - Pre-build Configuration Check")
    
    # Check if config.json exists
    if not os.path.exists(config_file):
        print(f"⚠️  config.json not found at: {config_file}")
        
        # First try to copy from private config directory
        if os.path.exists(private_config_file):
            print(f"📋 Copying from private config: {private_config_file}")
            shutil.copy2(private_config_file, config_file)
            print(f"✅ Created config.json from private config")
        # Fallback to example if private config doesn't exist
        elif os.path.exists(config_example):
            print(f"📋 Copying config.example.json to config.json...")
            shutil.copy2(config_example, config_file)
            print(f"✅ Created config.json from template")
            print()
            print("🔧 IMPORTANT: Edit data/config.json with your WiFi credentials:")
            print('   {"wifi": {"ssid": "YOUR_SSID", "password": "YOUR_PASSWORD"}}')
            print()
        else:
            print(f"❌ No config source found!")
            print(f"Please create either:")
            print(f"  - {private_config_file} (with real credentials)")
            print(f"  - {config_example} (template)")
            env.Exit(1)
    else:
        print(f"✅ config.json found")

# Run the check
check_config()