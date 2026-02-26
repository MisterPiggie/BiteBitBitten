// package main
//
// import (
// 	"fmt"
// 	"os"
// 	"strconv"
// )
//
//
// func (p *Parser) decode() (any){
// 	if p.err != nil {
// 		return nil
// 	}
// 	if p.pos >= len(p.file) {
//         p.err = fmt.Errorf("unexpected end of file at pos %d", p.pos)
// 		return nil
// 	}
// 	switch{
// 	case p.file[p.pos] == 'l':
// 		p.pos++
// 		var list []any
// 		for p.file[p.pos] != 'e' {
// 			val:= p.decode()
// 			if p.err != nil {    
// 				return nil
// 			}
// 			list = append(list, val)
// 		}
// 		p.pos++
// 		return list
// 	case p.file[p.pos] == 'i':
// 		end := indexOf(p.file, 'e', p.pos+1)	
// 		if end == -1 {
// 			p.err = fmt.Errorf("integer not closed at %d", p.pos)
// 			return nil
// 		}
//
// 		n, err := strconv.Atoi(string(p.file[p.pos+1:end]))
// 		if err != nil {
// 			p.err = fmt.Errorf("not a valid integer at %d err: %v", p.pos, err)
// 			return nil
// 		}
//
// 		p.pos = end + 1
//
// 		return n
// 	case p.file[p.pos] == 'd':
// 		p.pos++
// 		dict := map[string]any{}
//
// 		for p.file[p.pos] != 'e' {
// 			key:= p.decode()
//
// 			if p.err != nil {
// 				return nil
// 			}
// 			if key == "info" {
// 				p.infoStart == p.pos
// 			}
// 			val:= p.decode()
// 			if p.err != nil {
// 				return nil
// 			}
// 			if val == "info" {
// 				p.infoEnd == p.pos
// 			}
// 			dict[key.(string)] = val
// 		}
// 		p.pos++
//
// 		return dict
// 	default:
// 		colon := indexOf(p.file, ':', p.pos)
// 		length, err := strconv.Atoi(string(p.file[p.pos:colon]))
// 		if err != nil {
// 			p.err = fmt.Errorf("not a valid integer at %d err: %v", p.pos, err)
// 			return nil
// 		}
//
// 		start := colon + 1
// 		p.pos = start + length
// 		return string(p.file[start: p.pos])
// 	}
// }
//
// func indexOf(file []byte, target byte, start int) int {
// 	for i := start; i < len(file); i++ {
// 		if file[i] == target {
// 			return i
// 		}
// 	}
//
// 	return -1
// }
//
// func (p *Parser) getTorrentFile(data map[string]any) (TorrentFile, error) {
// 	var torrent TorrentFile
//
// 	announce, ok := data["announce"].(string)
// 	if ok{
// 		torrent.Announce = announce
// 	}
//
// 	announceList, ok := data["announce-list"].([]any)
// 	if ok {
// 		for _, tier := range announceList {
// 			var trackers []string
// 			for _, tracker := range tier.([]any){ 
// 				trackers = append(trackers, tracker.(string))
// 			}
// 			torrent.AnnounceList = append(torrent.AnnounceList, trackers)
// 		}
// 	}
//
// 	if torrent.Announce == "" && len(torrent.AnnounceList) == 0 {
// 		return TorrentFile{}, fmt.Errorf("missing announce")
// 	}
//
// 	infoMap, ok := data["info"].(map[string]any)
// 	if !ok {
// 		return TorrentFile{}, fmt.Errorf("missing info map")
// 	}
//
// 	torrent.Info.Name, ok = infoMap["name"].(string)
// 	if !ok {
// 		torrent.Info.Name = "no_name_was_provide"
// 	}
//
// 	torrent.Info.PieceLength, ok = infoMap["piece length"].(int)
// 	if !ok {
// 		return TorrentFile{}, fmt.Errorf("missing piece length")
// 	}
// 	torrent.Info.Pieces = []byte(infoMap["pieces"].(string))
// 	if len(torrent.Info.Pieces) == 0 {
// 		return TorrentFile{}, fmt.Errorf("missing pieces slice")
// 	}
//
// 	length, ok := infoMap["length"].(int) 
// 	if ok {
// 		torrent.Info.Files = []File{File{
// 			Length: length,
// 			Path: []string{torrent.Info.Name},
// 		}}
// 	} else if files, ok := infoMap["files"].([]any); ok {
// 		for _, file := range files {
// 			fileMap := file.(map[string]any)
// 			f := File{
// 				Length: fileMap["length"].(int),
// 			}
// 			for _, p := range fileMap["path"].([]any) {
// 				f.Path = append(f.Path, p.(string))
// 			}
//
// 			torrent.Info.Files = append(torrent.Info.Files, f)
// 		}
// 	} else {
// 		return TorrentFile{}, fmt.Errorf("no files provided")
// 	}
//
// 	torrent.Comment = data["comment"].(string)
// 	torrent.CreatedBy= data["created by"].(string)
// 	torrent.CreationDate= data["creation date"].(int)
// 	torrent.Publisher= data["publisher"].(string)
// 	torrent.PublisherURL= data["publisher-url"].(string)
// 	return torrent, nil	
//
//
//
// }
//
// func NewParser() *Parser {
// 	return &Parser{}
// }
//
// func main() {
// 	parser := NewParser()
// 	data, err := os.ReadFile("HadesII.torrent")
// 	if err != nil {
// 		return
// 	}
// 	parser.file = data
// 	rawData := parser.decode().(map[string]any)
// 	if parser.err != nil {
// 		return
// 	}
//
// 	torrent, err := parser.getTorrentFile(rawData)
// 	if err != nil {
// 		return
// 	}
//
// 	fmt.Println(torrent.Announce)
// 	fmt.Println(torrent.AnnounceList)
// 	fmt.Println(torrent.Publisher)
// 	fmt.Println(torrent.PublisherURL)
// 	fmt.Println(torrent.CreationDate)
// 	fmt.Println(torrent.Comment)
// 	fmt.Println(torrent.CreatedBy)
// 	fmt.Println(torrent.Info.Name)
// 	fmt.Println(torrent.Info.PieceLength)
// 	fmt.Println(torrent.Info.Pieces)
// 	fmt.Println(torrent.Info.Files)
//
// }
//

