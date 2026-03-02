package main

import (
	"fmt"
	"os"
	"github.com/MisterPiggie/BiteBitBitten/bencode"
)



func main() {
	file, _ := os.ReadFile("single.torrent")
	var t TorrentMeta
	if err := Unmarshal(file, &t); err != nil {
        panic(err)
    }

    fmt.Println("Announce:    ", t.Announce)
    fmt.Println("Name:        ", t.Info.Name)
    fmt.Println("Piece length:", t.Info.PieceLength)
    fmt.Println("Pieces len:  ", len(t.Info.Pieces))

    fmt.Println("\nAnnounce list:")
    for i, tier := range t.AnnounceList {
        fmt.Printf("  tier %d: %v\n", i, tier)
    }

    if t.Info.Length > 0 {
        fmt.Println("\nSingle file torrent")
        fmt.Println("Length:", t.Info.Length)
    } else {
        fmt.Println("\nMulti file torrent")
        for _, f := range t.Info.Files {
            fmt.Printf("  %v — %d bytes\n", f.Path, f.Length)
        }
    }
}
