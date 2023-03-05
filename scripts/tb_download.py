#  BlackCore is a chess engine
#  Copyright (c) 2023 SzilBalazs
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <https://www.gnu.org/licenses/>.

import urllib.request
import re

man5 = "https://tablebase.lichess.ovh/tables/standard/3-4-5/"
man6 = "https://tablebase.lichess.ovh/tables/standard/6-wdl/"


def dl_tablebase(url):
    print(f"Downloading {url} index...")
    urllib.request.urlretrieve(url, "index.html")
    print("Indexing files...")
    f = open("index.html", "r")
    lines = f.readlines()
    f.close()
    endgames = []
    for line in lines:
        if line.startswith("<a"):
            match = re.search(r'href=[\'"]?([^\'" >]+)', line)
            endgames.append(match.group(1))

    print(f"Found {len(endgames)} files!")

    for eg in endgames:
        print(f"Downloading {eg}...")
        urllib.request.urlretrieve(f"{url}{eg}", f"{eg}")


print("Downloading 6 man...")
dl_tablebase(man6)
