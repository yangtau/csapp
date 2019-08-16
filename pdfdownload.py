import bs4
import requests
from bs4 import BeautifulSoup
import os

base_url = 'http://www.cs.cmu.edu/~213/'
url = 'http://www.cs.cmu.edu/~213/schedule.html'
r = requests.get(url)
soup = BeautifulSoup(r.content)

links = soup.find_all('a')
links = [i.get('href') for i in links]
links = [i for i in links if i.endswith('.pdf')]

def skip_files():
   files = os.listdir()
   for f in files:
       if os.path.isdir(f):
           files += [f + '/' + i for i in os.listdir(f)]
   return files

def download(base, links, skip=[]):
    for url in links:
        if url in skip:
            print("skip %s" % url)
            continue
        r = requests.get(base+url)
        with open(url, "wb") as f:
            f.write(r.content)
            print('download %s' % url)

download(base_url, links, skip_files())
