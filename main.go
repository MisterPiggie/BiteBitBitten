package main

import (
	"bufio"
	"os"
	"reflect"
	"unsafe"
)

type TestSubject struct {
	Name 		string
	Age 		int
	Friends		[]string
	Dict		map[string]string
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

func main() {
	file, err := os.ReadFile("test_data.torrent")
	if err != nil {
		return
	}
	
	var testSubject TestSubject

	err := BencodeUnmarshal(file, &testSubject)
	if err != nil {
		return
	}
}


func buildCache(v any) map[string]fieldInfo {
	t := reflect.TypeOf(v)
	if t.Kind() == reflect.Ptr {
		t = t.Elem()



