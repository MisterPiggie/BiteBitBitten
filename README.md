## Status

Paused, but not abandoned — picking this back up eventually.

**Working:**
- .torrent file parsing (bencode)
- CLI interface
- resume file support
- piece hash verification (SHA-1)
- bitfield tracking
- disk space preallocation
- file path sanitization
- writing verified pieces to disk

**Roadblocks:**
- Need to redesign the peer model — right now peers drive downloads,
  but it should be pieces (or even blocks) driving which peer gets
  asked for what. Peer-driven doesn't parallelize well across peers.
- Memory preallocation is heavier than it should be. This was a
  conscious tradeoff to get something working, needs revisiting.
- Bugs in piece retrieval itself, on top of the above.
- File structure needs rethinking for better modularity.
