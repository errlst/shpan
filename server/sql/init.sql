-- 文件引用表
-- file_path 为系统中文件的路径，存储路径为 raws/${hash}_${filesize}
create table
    file_ref_table (
        id integer primary key autoincrement,
        ref_count integer not null default 0,
        file_path text not null unique
    );

-- 需要被删除的文件路径
create table
    file_to_del_table (file_path text primary key);

-- 引用计数为0时自动删除，并将文件路径插入到 file_to_del_table
create trigger del_ref_when_count_zero after
update of ref_count on file_ref_table for each row when NEW.ref_count = 0 begin
insert into
    file_to_del_table (file_path)
values
    (NEW.file_path);

delete from file_ref_table
where
    id = NEW.id;

end;

-- 条目共享链接
-- link 为随机生成的32位ascii字符串，降低撞库破解风险
-- left_times 为剩下有效的访问次数
create table
    shared_link_table (
        link text not null unique,
        entry_id integer not null,
        left_times integer not null
    );

-- 剩余访问次数为0后，删除link
create trigger del_shared_if_left_times_zero after
update of left_times on shared_link_table for each row when NEW.left_times = 0 begin
update entry_table
set
    shared_link = ''
where
    id = NEW.entry_id;

delete from shared_link_table
where
    link = NEW.link;

select
    raise (fail, "times_zero");

end;

-- 条目表
-- 如果 ref_id == 0，就是目录
create table
    entry_table (
        id integer primary key autoincrement,
        ref_id integer not null,
        path text not null unique, -- 用户空间路径，命名规则为 usr/xxx
        shared_link text default ''
    );

-- 父子条目关系
create table
    entry_rel_table (
        p_id integer not null,
        c_id integer not null,
        constraint unique_p_c_ UNIQUE (p_id, c_id)
    );

-- 添加 entry 时，如果不是目录，增加文件引用数
create trigger inc_ref_when_ins_entry after insert on entry_table for each row when NEW.ref_id != 0 begin
update file_ref_table
set
    ref_count = ref_count + 1
where
    id = NEW.ref_id;

end;

-- 删除 entry 时，删除对应的共享连接
create trigger del_shared_when_del_entry after delete on entry_table for each row when OLD.shared_link != '' begin
delete from shared_link_table
where
    link = OLD.shared_link;

end;

-- 删除文件 entry 时，减少文件引用数 和 关系
create trigger dec_ref_when_del_entry after delete on entry_table for each row when OLD.ref_id != 0 begin
update file_ref_table
set
    ref_count = ref_count - 1
where
    id = OLD.ref_id;

delete from entry_rel_table
where
    c_id = OLD.id;

end;

-- 删除目录 entry 时，删除所有的子 entry
create trigger del_rel_when_del_entry after delete on entry_table for each row when OLD.ref_id = 0 begin
delete from entry_table
where
    id in (
        select
            c_id
        from
            entry_rel_table
        where
            p_id = OLD.id
    );

delete from entry_rel_table
where
    p_id = OLD.id;

end;

-- 用户表
create table
    user_table (
        email text not null unique,
        passwd text not null,
        root_entry_id integer not null
    );