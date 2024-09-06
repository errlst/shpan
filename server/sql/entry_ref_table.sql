CREATE TABLE
    files (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        file_path TEXT NOT NULL,
        ref_count INTEGER NOT NULL
    );


-- 当 ref_count 为 0 时，自动删除该条目，并抛出提示
CREATE TRIGGER delete_when_ref_count_zero AFTER
UPDATE OF ref_count ON files FOR EACH ROW WHEN NEW.ref_count = 0 BEGIN
DELETE FROM files
WHERE
    id = NEW.id;
SELECT
    RAISE (FAIL, 'Record deleted: ref_count is zero.');
END;