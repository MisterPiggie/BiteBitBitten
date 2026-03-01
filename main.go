package main

import (
	"fmt"
	"os"
	"reflect"
	"sync"
	"unsafe"
)

type TorrentMeta struct {
	Announce		string				`bencode:"announce"`
	AnnounceList 	[][]string		`bencode:"announce-list"`
	Info			Info		`bencode:"info"`
}

type Info struct {
	Name		string		`bencode:"name"`
	PieceLength int		`bencode:"piece length"`
	Pieces		string		`bencode:"pieces"`
	Priva		int		`bencode:"priva"`
	Length 		int		`bencode:"length"`
	Files		[]File		`bencode:"files"`
}

type File struct {
	Path		[]string		`bencode:"path"`
	Length		int		`bencode:"length"`
}


type fieldDecoder struct {
	offset 		uintptr
	write 		func(unsafe.Pointer, *Tokenizer) error
}
var globalCache sync.Map 

func Unmarshal(data []byte, v any) error {
    t := reflect.TypeOf(v)
    if t.Kind() != reflect.Ptr {
        return fmt.Errorf("v must be a pointer")
    }
    t = t.Elem()

    var cache map[string]fieldDecoder
    if c, ok := globalCache.Load(t); ok {
        cache = c.(map[string]fieldDecoder)
    } else {
        var err error
        cache, err = buildCache(v)
        if err != nil {
            return fmt.Errorf("building cache: %w", err)
        }
        globalCache.Store(t, cache)
    }

    tok := NewTokenizer(data)
    return decodeStruct(unsafe.Pointer(reflect.ValueOf(v).Pointer()), tok, cache)
}

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
