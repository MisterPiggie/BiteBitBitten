package main

import (
	"unsafe"
)

type TorrentMeta struct {
	Announce		string
	AnnounceList 	[][]string
	Info			Info
}

type Info struct {
	Name		string
	PieceLength int
	Pieces		string
	Priva		int
	Length 		int
	Files		[]File
}

type File struct {
	Path		[]string
	Length		int
}

type BencodeValue struct {
	kind 		byte
	intVal		int
	strVal		string
	listVal		[]BencodeValue
	dictVal		map[string]BencodeValue
}

type fieldDecoder struct {
	offset 		uintptr
	decode 		func(unsafe.Pointer, []byte)
}

