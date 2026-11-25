#pragma once

// Copy this file to include/config.h and fill in your credentials.
// include/config.h is gitignored and must never be committed.

#ifndef WIFI_SSID
#define WIFI_SSID "your-ssid"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "your-password"
#endif

#ifndef CAR_HOSTNAME
#define CAR_HOSTNAME "thushimas-car"
#endif

// Shared secret sent in the X-Control-Token header. Use a long random value.
#ifndef CONTROL_TOKEN
#define CONTROL_TOKEN "replace-with-a-long-random-token"
#endif
