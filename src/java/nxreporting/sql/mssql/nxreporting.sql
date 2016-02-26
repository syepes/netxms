
    -- drop table report_notifications;

    -- drop table report_results;

    create table report_notifications (
        id int not null,
        attach_format_code int,
        jobid varchar(255),
        mail varchar(255),
        report_name varchar(255),
        primary key (id)
    );

    create table report_results (
        id int not null,
        executionTime numeric(19,0),
        jobId varchar(255),
        reportId varchar(255),
        userId int,
        primary key (id)
    );
