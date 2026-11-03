import hashlib
import os
import sys
import time
import urllib.request

URL = "https://cdimage.ubuntu.com/ubuntu-base/releases/24.04/release/ubuntu-base-24.04.5-base-amd64.tar.gz"
EXPECT_SHA256 = "e77b6f10c2590cef872b33ee9f635a0e3fd1f57fb074c0e52b5c7f56147a0c86"
OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "downloads")
OUT = os.path.join(OUT_DIR, "ubuntu-base-24.04.5-base-amd64.tar.gz")
PART = OUT + ".part"


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    req = urllib.request.Request(URL, headers={"User-Agent": "Mozilla/5.0"})
    t0 = time.time()
    n = 0
    h = hashlib.sha256()
    with urllib.request.urlopen(req, timeout=60) as r, open(PART, "wb") as f:
        total = int(r.headers.get("Content-Length") or 0)
        while True:
            chunk = r.read(1024 * 1024)
            if not chunk:
                break
            f.write(chunk)
            h.update(chunk)
            n += len(chunk)
            if n % (5 * 1024 * 1024) < 1024 * 1024:
                speed = n / 1048576 / max(time.time() - t0, 0.001)
                print("%.0f/%.0f MB  %.1f MB/s" % (n / 1048576, total / 1048576, speed), flush=True)
    got = h.hexdigest()
    print("size  =", n)
    print("sha256=", got)
    print("expect=", EXPECT_SHA256)
    if got.lower() == EXPECT_SHA256.lower():
        os.replace(PART, OUT)
        print("OK saved to", OUT)
        return 0
    print("HASH MISMATCH, kept as", PART)
    return 2


if __name__ == "__main__":
    sys.exit(main())
