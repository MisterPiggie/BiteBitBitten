package main

import (
	"fmt"
	"os"
	"strconv"
)

type Parser struct {
	file 		[]byte
	pos			int
	err			error
}

type Torrent struct {
	Announce		string
	AnnounceList 	[]string
	Publisher		string
	PublisherURL	string
	CreationDate	int
	Comment 		string
	CreatedBy		string
	Info 			map[string]any
}


func (p *Parser) decode() (any){
	if p.err != nil {
		return nil
	}
	if p.pos >= len(p.file) {
        p.err = fmt.Errorf("unexpected end of file at pos %d", p.pos)
		return nil
	}
	switch{
	case p.file[p.pos] == 'l':
		p.pos++
		var list []any
		for p.file[p.pos] != 'e' {
			val:= p.decode()
			if p.err != nil {    
				return nil
			}
			list = append(list, val)
		}
		p.pos++
		return list
	case p.file[p.pos] == 'i':
		end := indexOf(p.file, 'e', p.pos+1)	
		if end == -1 {
			p.err = fmt.Errorf("integer not closed at %d", p.pos)
			return nil
		}

		n, err := strconv.Atoi(string(p.file[p.pos+1:end]))
		if err != nil {
			p.err = fmt.Errorf("not a valid integer at %d err: %v", p.pos, err)
			return nil
		}

		p.pos = end + 1

		return n
	case p.file[p.pos] == 'd':
		p.pos++
		dict := map[string]any{}

		for p.file[p.pos] != 'e' {
			key:= p.decode()
			if p.err != nil {
				return nil
			}
			val:= p.decode()
			if p.err != nil {
				return nil
			}
			dict[key.(string)] = val
		}
		p.pos++

		return dict
	default:
		colon := indexOf(p.file, ':', p.pos)
		length, err := strconv.Atoi(string(p.file[p.pos:colon]))
		if err != nil {
			p.err = fmt.Errorf("not a valid integer at %d err: %v", p.pos, err)
			return nil
		}

		start := colon + 1
		p.pos = start + length
		return string(p.file[start: p.pos])
	}
}

func indexOf(file []byte, target byte, start int) int {
	for i := start; i < len(file); i++ {
		if file[i] == target {
			return i
		}
	}

	return -1
}

func (p *Parser) getTorrentFile(filePath string) (map[string]any, error) {
	file, err := os.ReadFile(filePath)
	if err != nil {
		return nil, err
	}

	p.file = file
	data := p.decode()
	if p.err != nil {
		return nil, err 
	}

	torrent, ok := data.(map[string]any) 
	if !ok {
		return nil, fmt.Errorf("error turning data into map")
	}
	torrentStruct := Torrent{}
	torrentStruct.Announce, ok = torrent["announce"].(string)
}
	
func NewParser() *Parser {
	return &Parser{}
}

func main() {
	parser := NewParser()
	torrent, err := parser.getTorrentFile("HadesII.torrent")
	if err != nil {
		return
	}
	fmt.Printf("%T\n",torrent["publisher"])
	fmt.Printf("%T\n",torrent["publisher-url"])
	fmt.Printf("%T\n",torrent["announce"])
	fmt.Printf("%T\n",torrent["announce-list"])
	fmt.Printf("%T\n",torrent["comment"])
	fmt.Printf("%T\n",torrent["created by"])
	fmt.Printf("%T\n",torrent["creation date"])
	fmt.Printf("%T\n",torrent["info"])
	info := torrent["info"].(map[string]any)
	fmt.Println(info["name"])
}



