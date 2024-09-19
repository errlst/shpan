-- 条目表
create table
    if not exists entry_table (
        id integer primary key autoincrement,
        ref_id integer default null,
        path txt not null unique, -- 用户空间路径
        shared_link text default null,
        is_directory boolean not null
    );

-- 文件引用表
-- file_path 为系统中文件的路径，存储路径为 raws/${hash}_${filesize}
create table
    if not exists file_ref_table (
        id integer primary key autoincrement,
        ref_count integer not null default 0,
        file_path text not null unique
    );

-- 引用计数为0时自动删除该条目，并抛出异常
create trigger if not exists delete_when_ref_count_zero after
update of ref_count on file_ref_table for each row when NEW.ref_count = 0 begin
delete from file_ref_table
where
    id = NEW.id;

select
    RAISE (FAIL, 'ref_count is zero');

end;

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