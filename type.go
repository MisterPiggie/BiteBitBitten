package main



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



