package main

import (
	"fmt"
	"go/token"
	"os"
	"strconv"
)

type Parser struct {
	file 		[]byte
	pos			int
	err			error
}

type TorrentFile struct {
	Announce		string
	AnnounceList 	[][]string
	Publisher		string
	PublisherURL	string
	CreationDate	int
	Comment 		string
	CreatedBy		string
	Info 			Info
}

type Info struct {
	Name		string
	PieceLength int
	Pieces		[]byte
	Files		[]File
}

type File struct {
	Length 		int
	Path 		[]string
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

func (p *Parser) getTorrentFile(data map[string]any) (map[string]any, error) {
	var torrent TorrentFile

	announce, ok := data["announce"].(string)
	if ok{
		torrent.Announce = announce
	}

	announceList, ok := data["announce-list"].([]any)
	if ok {
		for _, tier := range announceList {
			var trackers []string
			for _, tracker := range tier.([]any){ 
				trackers = append(trackers, tracker.(string))
			}
			torrent.AnnounceList = append(torrent.AnnounceList, trackers)
		}
	}

	if torrent.Announce == "" && len(torrent.AnnounceList) == 0 {
		return nil, fmt.Errorf("missing announce")
	}

	infoMap, ok := data["info"].(map[string]any)
	if !ok {
		return nil, fmt.Errorf("missing info map")
	}

	torrent.Info.Name = infoMap["name"].(string)
	torrent.Info.PieceLength, ok = infoMap["piece length"].(int)
	if !ok {
		return nil, fmt.Errorf("missing piece length")
	}
	torrent.Info.Pieces = []byte(infoMap["pieces"].(string))
	if len(token.Info.Pieces) == 0 {
		return nil, fmt.Errorf("missing pieces slice")
	}

	torrent.Info.Length, ok = infoMap["length"].(int) 
	if !ok {
		files, ok := infoMap["files"].([]any)
		if ok {
			for _, file := range files {
				fileMap := file.(map[string]any)
				f := File{
					Length: fileMap["length"].(int),
				}
				for _, p := range fileMap["path"].([]any) {
					file.Path = append(file.Path, p.(string))
				}
				info


}

func NewParser() *Parser {
	return &Parser{}
}

func main() {
	parser := NewParser()
	data, err := os.ReadFile("HadesII.torrent")
	if err != nil {
		return
	}
	parser.file = data
	torrent := parser.decode().(map[string]any)
	if parser.err != nil {
		return
	}
	fmt.Printf("%T\n",torrent["publisher"])
	fmt.Printf("%T\n",torrent["publisher-url"])
	fmt.Printf("%T\n",torrent["announce"])
	fmt.Printf("%v\n",torrent["announce-list"])
	fmt.Printf("%T\n",torrent["comment"])
	fmt.Printf("%T\n",torrent["created by"])
	fmt.Printf("%T\n",torrent["creation date"])
	fmt.Printf("%T\n",torrent["info"])
}



