create table studenti (
   student integer primary key,
   jmeno text,
   prijmeni text,
   studuje integer default 1 check (studuje in (0,1)),
   img_base64 text
);

create index index_prijmeni on studenti (prijmeni);

create table predmety (
   predmet text primary key,
   aktivni integer default 1 check (aktivni in (0,1))
);

create table zapisy (
   predmet text,
   student integer,
   primary key (predmet, student),
   foreign key (predmet) references predmety (predmet),
   foreign key (student) references studenti (student)
);

create table hodnoceni (
   predmet text,
   student integer,
   datum   text,
   body    integer not null default 0 check (body >= -1),
   primary key (predmet, student, datum),
   foreign key (predmet) references predmety (predmet),
   foreign key (student) references studenti (student),
   foreign key (predmet, student) references zapisy (predmet, student)
);
