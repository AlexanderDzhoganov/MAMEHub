set -e
scp csume.exe www-data@10ghost.net:/var/www/MAMEHubDownloads/Emulators/Windows/
scp csume.sym www-data@10ghost.net:/var/www/MAMEHubDownloads/Emulators/Windows/
ssh www-data@10ghost.net gzip -f /var/www/MAMEHubDownloads/Emulators/Windows/csume.exe
ssh www-data@10ghost.net gzip -f /var/www/MAMEHubDownloads/Emulators/Windows/csume.sym