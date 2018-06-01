## SYNTAX

#### select

- [ ] select `[属性名/*]` from `表名` [where `条件`];

#### create

- [ ] create table `表名` (

  ​	`属性名` `类型` [`unique/primary key`],

  ​	`属性名` `类型` [`unique/primary key`],

  ​	...

  ​	`属性名` `类型` [`unique/primary key`],

  ​	[`primary key (属性名)`]

  );

- [ ] create index `索引名` on `表名` (`属性名`);

- [ ] [createdb `数据库名` ;]

#### drop

- [ ] drop table `表名` ;
- [ ] drop index `索引名` ; 
- [ ] [dropdb `数据库名` ;]

#### delete

- [ ] delete from `表名` [where `条件`];

#### insert

- [ ] insert into `表名` values `(值1， 值2， ... , 值n)` ;
- [ ] insert into `表名(属性1, 属性2, ... , 属性n)` values `(值1， 值2， ... , 值n)` ;

#### update

- [ ] 

#### execfile

- [ ] execfile `文件名` ;

#### help

- [ ] [help/doc/man]

#### quit

- [ ] quit

#### [system statement]

- [ ] \dt （显示所有的表名）

- [ ] \dt `表名` (显示该表的属性名，属性type， 约束条件)

- [ ] \c (显示当前所连接的数据库的名称以及用户名)

  > minisql=# \c
  > You are now connected to database "minisql" as user "admin".

- [ ] \c `数据库名` （切换数据库）

- [ ] \? 帮助文档

- [ ] \i `文件名` (处理文件)

- [ ] \q 退出

---

#### type

- [ ] int
- [ ] char(n) `1 <= n <= 255，需要检查n的大小`
- [ ] float

#### condition

- [ ] `属性名1` op `值1` [bool_op `属性名2` op `值2` [bool_op...]]
- arith_op包括（=, <, <=, >, >=, <>）
- bool_op包括（and, [or, not]）

---

输入提示符

[database_name =>]处理当前行

[database_name ->]处于多行处理中

---

```c++
enum
{
	SYNTAX_ERROR,
    INSERT,
    CREATE_TABLE,
    CREATE_INDEX,
    CREATE_DATABASE,
    DROP_TABLE,
    DROP_INDEX,
    DROP_DATABASE,
    DELETE,
    INSERT,
    UPDATE,
    EXECFILE,
    HELP,
    QUIT,
    // 下面是我瞎想的，实现也比较简单，只要和catalog manager交互就可以了。
    /*
    SYSTEM_LIST_TABLE,
    SYSTEM_LIST_TABLE_ATTRIBUTES,
    SYSTEM_SHOW_LOGIN_INFO,
    SYSTEM_HELP,
    SYSTEM_EXECFILE,
    SYSTEM_QUIT,
    */
}
/*
参照方老板的，感觉interpreter可以只是一个类就好啦，毕竟输入的syntax是固定的，我们对每一个开头的字符判断，初步判定类型语句类型，然后再丢到各自的语句函数里面再次检测syntax。如果是没问题的再往API里面传，传分隔之后的属性，表名/属性名/值/条件/TYPE，或者API也设计多个函数去做处理也没问题。
然后用stringstream来存控制台输入流或者文件输入流。然后再去处理分隔。
*/
```

