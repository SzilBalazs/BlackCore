import urllib.request
import re

man5 = "https://tablebase.lichess.ovh/tables/standard/3-4-5/"
man6wdl = "https://tablebase.lichess.ovh/tables/standard/6-wdl/"
man6dtz = "https://tablebase.lichess.ovh/tables/standard/6-dtz/"


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
dl_tablebase(man6dtz)
