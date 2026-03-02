package main_test

import (
	"bytes"
	"os"
	"testing"

	"github.com/MisterPiggie/BiteBitBitten/bencode"
	jackpal "github.com/jackpal/bencode-go"
	zeebo "github.com/zeebo/bencode"
)


type File struct {
	Length int    `bencode:"length"`
	Path   []string `bencode:"path"`
}

type Info struct {
	Name        string `bencode:"name"`
	PieceLength int  `bencode:"piece length"`
	Pieces      string `bencode:"pieces"`
	Length      int  `bencode:"length"`
	Files       []File `bencode:"files"`
	Private     int    `bencode:"private"`
}

type Torrent struct {
	Announce     string     `bencode:"announce"`
	AnnounceList [][]string `bencode:"announce-list"`
	Info         Info       `bencode:"info"`
}


var (
	singleData []byte
	multiData  []byte
)

func init() {
	var err error
	singleData, err = os.ReadFile("single.torrent")
	if err != nil {
		panic("missing single.torrent: " + err.Error())
	}
	multiData, err = os.ReadFile("multi.torrent")
	if err != nil {
		panic("missing multi.torrent: " + err.Error())
	}
}

// ---- your decoder ----

func BenchmarkYours_Single(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		var t Torrent
		if err := bencode.Unmarshal(singleData, &t); err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkYours_Multi(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		var t Torrent
		if err := bencode.Unmarshal(multiData, &t); err != nil {
			b.Fatal(err)
		}
	}
}

// ---- jackpal/bencode-go ----

func BenchmarkJackpal_Single(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		var t Torrent
		r := bytes.NewReader(singleData)
		if err := jackpal.Unmarshal(r, &t); err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkJackpal_Multi(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		var t Torrent
		r := bytes.NewReader(multiData)
		if err := jackpal.Unmarshal(r, &t); err != nil {
			b.Fatal(err)
		}
	}
}

// ---- zeebo/bencode ----

func BenchmarkZeebo_Single(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		var t Torrent
		if err := zeebo.DecodeBytes(singleData, &t); err != nil {
			b.Fatal(err)
		}
	}
}

func BenchmarkZeebo_Multi(b *testing.B) {
	b.ReportAllocs()
	for i := 0; i < b.N; i++ {
		var t Torrent
		if err := zeebo.DecodeBytes(multiData, &t); err != nil {
			b.Fatal(err)
		}
	}
}

// ---- correctness test — all decoders should give same result ----

func TestCorrectness(t *testing.T) {
	var yours, jack, zee Torrent

	// yours
	if err := bencode.Unmarshal(singleData, &yours); err != nil {
		t.Fatalf("yours failed: %v", err)
	}

	// jackpal
	if err := jackpal.Unmarshal(bytes.NewReader(singleData), &jack); err != nil {
		t.Fatalf("jackpal failed: %v", err)
	}

	// zeebo
	if err := zeebo.DecodeBytes(singleData, &zee); err != nil {
		t.Fatalf("zeebo failed: %v", err)
	}

	// compare
	if yours.Announce != jack.Announce {
		t.Errorf("announce mismatch: yours=%q jackpal=%q", yours.Announce, jack.Announce)
	}
	if yours.Announce != zee.Announce {
		t.Errorf("announce mismatch: yours=%q zeebo=%q", yours.Announce, zee.Announce)
	}
	if yours.Info.Name != jack.Info.Name {
		t.Errorf("info.name mismatch: yours=%q jackpal=%q", yours.Info.Name, jack.Info.Name)
	}
	if yours.Info.PieceLength != jack.Info.PieceLength {
		t.Errorf("piece length mismatch: yours=%d jackpal=%d", yours.Info.PieceLength, jack.Info.PieceLength)
	}
	if yours.Info.Length != jack.Info.Length {
		t.Errorf("length mismatch: yours=%d jackpal=%d", yours.Info.Length, jack.Info.Length)
	}
	if len(yours.AnnounceList) != len(jack.AnnounceList) {
		t.Errorf("announce-list length mismatch: yours=%d jackpal=%d", len(yours.AnnounceList), len(jack.AnnounceList))
	}

	t.Logf("all decoders agree on single.torrent ✓")
	t.Logf("announce:     %s", yours.Announce)
	t.Logf("name:         %s", yours.Info.Name)
	t.Logf("piece length: %d", yours.Info.PieceLength)
	t.Logf("length:       %d", yours.Info.Length)
	t.Logf("announce-list tiers: %d", len(yours.AnnounceList))

	// test multi
	var yoursM, jackM Torrent
	if err := bencode.Unmarshal(multiData, &yoursM); err != nil {
		t.Fatalf("yours multi failed: %v", err)
	}
	if err := jackpal.Unmarshal(bytes.NewReader(multiData), &jackM); err != nil {
		t.Fatalf("jackpal multi failed: %v", err)
	}
	if len(yoursM.Info.Files) != len(jackM.Info.Files) {
		t.Errorf("files length mismatch: yours=%d jackpal=%d", len(yoursM.Info.Files), len(jackM.Info.Files))
	}
	for i, f := range yoursM.Info.Files {
		if f.Length != jackM.Info.Files[i].Length {
			t.Errorf("file[%d] length mismatch: yours=%d jackpal=%d", i, f.Length, jackM.Info.Files[i].Length)
		}
	}
	t.Logf("multi.torrent files: %d ✓", len(yoursM.Info.Files))
}
