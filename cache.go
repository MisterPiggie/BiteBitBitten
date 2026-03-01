package main

import (
	"fmt"
	"reflect"
	"unsafe"
)

func buildCache(v any) (map[string]fieldDecoder, error) {
	t := reflect.TypeOf(v)
	if t.Kind() == reflect.Ptr {
		t = t.Elem()
	}

	cache := map[string]fieldDecoder{}

	for i := 0; i < t.NumField(); i++ {
		field := t.Field(i)

		key := field.Tag.Get("bencode")
		if key == "" {
			key = field.Name
		}

		writeFunc, err := resolveWriter(field.Type)
		if err != nil {
			return nil, fmt.Errorf("field %v: %v", field.Name, err)
		}

		cache[key] = fieldDecoder{
			offset: field.Offset,
			write:  writeFunc,
		}
	}
	return cache, nil
}

func resolveWriter(t reflect.Type) (func(unsafe.Pointer, *Tokenizer) error, error) {
	switch t.Kind() {

	case reflect.String:
		return func(ptr unsafe.Pointer, tok *Tokenizer) error {
			s, err := tok.ReadString()
			if err != nil {
				return err
			}
			*(*string)(ptr) = s
			return nil
		}, nil
	case reflect.Int:
		return func(ptr unsafe.Pointer, tok *Tokenizer) error {
			num, err := tok.ReadInt()
			if err != nil {
				return err
			}
			*(*int64)(ptr) = int64(num)

			return nil
		}, nil

	case reflect.Slice:
		funcSlice, err := caseSlice(t.Elem())
		if err != nil {
			return nil, err
		}
		return funcSlice, nil

	case reflect.Struct:
		funcStruct, err := caseStruct(t)
		if err != nil {
			return nil, err
		}
		return funcStruct, nil

	default:
		return nil, fmt.Errorf("unsupported bencode field type")
	}
}


func caseSlice(elem reflect.Type) (func (unsafe.Pointer, *Tokenizer) error, error) {

	switch elem.Kind() {
	case reflect.String:
		return func(ptr unsafe.Pointer, tok *Tokenizer) error {
			err := tok.ReadOpen();
			if  err != nil {
				return err
			}

			var result []string 
			for !tok.AtEnd() {
				s, err := tok.ReadString()
				if err != nil {
					return err
				}
				result = append(result, s)
			}
			*(*[]string)(ptr) = result
			return tok.ReadEnd()
		}, nil
	case reflect.Int, reflect.Int64:
		return func(ptr unsafe.Pointer, tok *Tokenizer) error {
			err := tok.ReadOpen()
			if err != nil {
				return err
			}

			var result []int64
			for !tok.AtEnd() {
				n, err := tok.ReadInt()
				if err != nil {
					return err
				}
				result = append(result, n)
			}
			*(*[]int64)(ptr) = result
			return tok.ReadEnd()
		}, nil

	case reflect.Slice:
		if elem.Elem().Kind() != reflect.String {
			return nil, fmt.Errorf("not yet implemented type of slice of slices: %s", elem.Elem())
		}

		return func(ptr unsafe.Pointer, tok *Tokenizer) error {

			err := tok.ReadOpen()
			if err != nil {
				return err
			}

			var result [][]string
			for !tok.AtEnd() {
				err := tok.ReadOpen()
				if err != nil {
					return err
				}
				var inner []string
				for !tok.AtEnd() {
					s, err := tok.ReadString()
					if err != nil {
						return err
					}

					inner = append(inner, s)
				}
				err = tok.ReadEnd()
				if err != nil {
					return err
				}
				result = append(result, inner)
			}
			*(*[][]string)(ptr) = result
			return tok.ReadEnd()
		}, nil

	case reflect.Struct:
		nested, err := buildCache(reflect.New(elem).Interface())
		if err != nil {
			return nil, fmt.Errorf("slice struct %s: %w", elem.Name(), err)
		}
		sliceType := reflect.SliceOf(elem)
		return func(ptr unsafe.Pointer, tok *Tokenizer) error {
			err := tok.ReadOpen()
			if  err != nil {
				return err
			}

			slice := reflect.MakeSlice(sliceType, 0, 4)
			for !tok.AtEnd() {
				elemVal := reflect.New(elem)
				err := decodeStruct(elemVal.UnsafePointer(), tok, nested)
				if  err != nil {
					return err
				}
				slice = reflect.Append(slice, elemVal.Elem())
			}

			err = tok.ReadEnd() 
			if err != nil {
				return err
			}

			reflect.NewAt(sliceType, ptr).Elem().Set(slice)
			return nil
		}, nil

	default:
		return nil, fmt.Errorf("unsupported slice element: %s", elem)
	}
}


func caseStruct(t reflect.Type) (func(unsafe.Pointer, *Tokenizer) error, error) {
	nested, err := buildCache(reflect.New(t).Interface())
	if err != nil {
		return nil, fmt.Errorf("struct %v: %v", t.Name(), err)
	}
	
	return func(p unsafe.Pointer, t *Tokenizer) error {
		return decodeStruct(p, t, nested)
	}, nil
}
