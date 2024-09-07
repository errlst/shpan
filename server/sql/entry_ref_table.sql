CREATE TABLE
    entry_ref_table (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        file_path TEXT NOT NULL,
        ref_count INTEGER NOT NULL DEFAULT 1
    );

CREATE TRIGGER delete_when_ref_count_zero AFTER
UPDATE OF ref_count ON entry_ref_table FOR EACH ROW WHEN NEW.ref_count = 0 BEGIN
DELETE FROM entry_ref_table
WHERE
    id = NEW.id;

SELECT
    RAISE (FAIL, 'ref_count is zero');

END;