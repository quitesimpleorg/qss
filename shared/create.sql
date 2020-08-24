CREATE TABLE file(id INTEGER PRIMARY KEY, path varchar(4096) UNIQUE, mtime integer, size integer, filetype char(1));
CREATE TABLE content(id INTEGER PRIMARY KEY, fileid INTEGER REFERENCES file (id) ON DELETE CASCADE, page integer, content text);
CREATE VIRTUAL TABLE content_fts USING fts5(content, content='content', content_rowid='id')
/* content_fts(content) */;
CREATE TRIGGER contents_ai AFTER INSERT ON content BEGIN
  INSERT INTO content_fts(rowid, content) VALUES (new.id, new.content);
END;
CREATE TRIGGER contents_ad AFTER DELETE ON content BEGIN
  INSERT INTO content_fts(content_fts, rowid, content) VALUES('delete', old.id, old.content);
END;
CREATE TRIGGER contents_au AFTER UPDATE ON content BEGIN
  INSERT INTO content_fts(content_fts, rowid, content) VALUES('delete', old.id, old.content);
  INSERT INTO content_fts(rowid, content) VALUES (new.id, new.content);
END;
