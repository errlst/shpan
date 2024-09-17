-- 条目表
CREATE TABLE
    entry_table (
        id INT AUTO_INCREMENT PRIMARY KEY,
        ref_id INT DEFAULT NULL,
        path TEXT NOT NULL, -- 用户空间路径
        shared_link TEXT DEFAULT NULL,
        is_directory BOOLEAN NOT NULL
    );

-- 文件引用表
CREATE TABLE
    file_ref_table (
        id INT AUTO_INCREMENT PRIMARY KEY,
        ref_count INT NOT NULL DEFAULT 1,
        file_path TEXT NOT NULL,
    );

-- 引用计数为0时自动删除该条目，并抛出异常
CREATE TRIGGER delete_when_ref_count_zero AFTER
UPDATE OF ref_count ON entry_ref_table FOR EACH ROW WHEN NEW.ref_count = 0 BEGIN
DELETE FROM entry_ref_table
WHERE
    id = NEW.id;

SELECT
    RAISE (FAIL, 'ref_count is zero');

END;

-- 条目共享链接
CREATE TABLE
    shared_link_table (
        link TEXT NOT NULL PRIMARY KEY,
        entry_id INT NOT NULL,
        FOREIGN KEY (entry_id) REFERENCES file_system (id)
    );

-- 用户表
CREATE TABLE
    user_table (
        email TEXT NOT NULL PRIMARY KEY,
        passwd TEXT NOT NULL,
        root_entry_id INT NOT NULL,
        FOREIGN KEY (root_entry_id) REFERENCES file_system (id)
    );