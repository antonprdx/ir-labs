# Лабораторная работа: Информационный поиск

**Студент:** Кабанов Антон Алексеевич  
**Группа:** М8О-401Б-22

## Описание

Проект представляет собой поисковую систему полного цикла. Реализованы:

- **Краулер (Python)** для сбора корпуса документов.
- **Индексатор (C++/C)** с собственной реализацией хэш-таблиц и векторов (без STL).
- **Поисковый движок (C++/C)** с поддержкой булевой логики (AND, OR).

## Структура проекта

```
.
├── crawler.py
├── README.md
├── search
│   ├── boolean_index.c
│   ├── boolean_index.h
│   ├── boolean_searcher.c
│   ├── boolean_searcher.h
│   ├── hashmap.c
│   ├── hashmap.h
│   ├── index_builder.c
│   ├── index_stats.c
│   ├── Makefile
│   ├── posting_list.c
│   ├── posting_list.h
│   ├── query_parser.c
│   ├── query_parser.h
│   ├── searcher.c
│   ├── string.c
│   ├── string.h
│   ├── vector.c
│   └── vector.h
├── sites.txt
├── stemmer.cpp
├── tokenizer.cpp
├── zipf.cpp
└── zipf.py

```

## Запуск

```bash
$ make
$ python crawler.py
$ ./index_builder <directory>
$ ./search_cli
```
