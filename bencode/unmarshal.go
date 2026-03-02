package bencode

import (
	"fmt"
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
