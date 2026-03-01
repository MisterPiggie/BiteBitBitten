package main

import (
	"bytes"
	"fmt"
	"strconv"
	"unsafe"
)


type TokenKind byte 


const (
	TokenString 	TokenKind = 's'
	TokenInt		TokenKind = 'i'
	TokenList		TokenKind = 'l'
	TokenDict		TokenKind = 'd'
	TokenEnd		TokenKind = 'e'
)


type Token struct {
	Kind 	TokenKind
	Str 	string
	Int 	int64
}


type Tokenizer struct {
	data	[]byte
	pos		int
}


func NewTokenizer(data []byte) *Tokenizer {
	return &Tokenizer{data: data, pos: 0,}
}

func (t *Tokenizer) Peek() TokenKind {
	if t.pos >= len(t.data) {
		return TokenEnd
	}

	switch {
		case t.data[t.pos] == 'i': return TokenInt
		case t.data[t.pos] == 'l': return TokenList
		case t.data[t.pos] == 'd': return TokenDict
		case t.data[t.pos] == 'e': return TokenEnd
		case t.data[t.pos] >= '0' && t.data[t.pos] <= '9': return TokenString
	}

	return TokenEnd
}


func (t *Tokenizer) ReadString() (string, error) {
	colon := bytes.IndexByte(t.data[t.pos:], ':')
	if colon < 0 {
		return "", fmt.Errorf("missing colon in string at %d", t.pos)
	}

	length, err := strconv.Atoi(string(t.data[t.pos:t.pos+colon]))
	if err != nil {
		return "", fmt.Errorf("malformed string length at %d", t.pos)
	}

	t.pos += colon+1

	if t.pos + length > len(t.data) {
		return "", fmt.Errorf("string too short at %d", t.pos)
	}

	s := string(t.data[t.pos:t.pos+length])
	t.pos += length
	return s, nil
}

func (t *Tokenizer) ReadInt() (int64, error) {
	if t.data[t.pos] != 'i' {
		return 0, fmt.Errorf("expected 'i' at pos %d", t.pos)
	}
	
	t.pos++

	end := bytes.IndexByte(t.data[t.pos:], 'e')
	if end < 0 {
		return 0, fmt.Errorf("missing 'e' in integer at pos %d", t.pos)
	}

	num, err := strconv.ParseInt(string(t.data[t.pos:t.pos+end]), 10, 64)
	if err != nil {
		return 0, fmt.Errorf("malformed integer at pos %d", t.pos)
	}

	t.pos += 1 + end 
	return num, nil
}

func (t *Tokenizer) ReadOpen() error {
	if t.data[t.pos] != 'l' && t.data[t.pos] != 'd' {
		return fmt.Errorf("expected 'l' or 'd' at pos %d", t.pos)
	}

	t.pos++
	return nil
}


func (t *Tokenizer) ReadEnd() error {
	if t.data[t.pos] != 'e' { 
		return fmt.Errorf("expected 'e' at pos %d", t.pos)
	}
	t.pos++
	return nil
}



func (t *Tokenizer) AtEnd() bool {
	return t.pos >= len(t.data) || t.data[t.pos] == 'e'
}

func decodeStruct(ptr unsafe.Pointer, tok *Tokenizer, cache map[string]fieldDecoder) error {
	err := tok.ReadOpen()
	if err != nil {
		return err
	}

	for !tok.AtEnd() {
		key, err := tok.ReadString()
		if err != nil {
			return err
		}

		c, ok := cache[key]
		if !ok {
			tok.Skip()
			continue
		}

		fieldPtr := unsafe.Pointer(uintptr(ptr) + c.offset) 
		err := c.write(fieldPtr, tok)
		if err != nil {
			return fmt.Errorf("field %s: %v", key, err)
		}
	}

	return tok.ReadEnd()
}

func (t *Tokenizer) Skip() error {
	if t.pos > len(t.data) {
		return fmt.Errorf("unexpected EOF")
	}
	switch t.Peek(){
	case TokenString:
		colon := bytes.IndexByte(t.data[t.pos:], ':')
		if colon < 0 {
			return fmt.Errorf("missing colon in string at %d", t.pos)
		}

		length, err := strconv.Atoi(string(t.data[t.pos:t.pos+colon]))
		if err != nil {
			return fmt.Errorf("malformed string length at %d", t.pos)
		}

		t.pos += colon + 1 + length

	case TokenInt:
		t.pos++

		for t.pos < len(t.data) && t.data[t.pos] != 'e' {
			t.pos++
		}
		t.pos++
	case TokenDict, TokenList:
		t.pos++
		for !t.AtEnd(){
			t.Skip()
		}
		t.pos++
	default:
		t.pos = len(t.data)
	}

	return nil
}


















