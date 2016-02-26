package com.radensolutions.reporting.model;

import org.hibernate.annotations.GenericGenerator;
import org.hibernate.annotations.Type;

import javax.persistence.*;
import java.io.Serializable;
import java.util.Date;
import java.util.UUID;

@Entity
@Table(name = "report_results")
public class ReportResult implements Serializable {

    private static final long serialVersionUID = -3946455080023055986L;

    @Id
    @Column(name = "id")
    @GenericGenerator(name = "report_results_pk_gen", strategy = "increment")
    @GeneratedValue(generator = "report_results_pk_gen")
    private Integer id;

    @Column(name = "executionTime")
    private Long executionTime;

    @Column(name = "reportId")
    @Type(type = "uuid-char")
    private UUID reportId;

    @Column(name = "jobId")
    @Type(type = "uuid-char")
    private UUID jobId;

    @Column(name = "userId")
    private int userId;

    public ReportResult() {
    }

    public ReportResult(Date executionTime, UUID reportId, UUID jobId, int userId) {
        this.executionTime = executionTime.getTime() / 1000;
        this.reportId = reportId;
        this.jobId = jobId;
        this.userId = userId;
    }

    public Integer getId() {
        return id;
    }

    public void setId(Integer id) {
        this.id = id;
    }

    public Date getExecutionTime() {
        return new Date(executionTime * 1000);
    }

    public void setExecutionTime(Date executionTime) {
        this.executionTime = executionTime.getTime() / 1000;
    }

    public UUID getReportId() {
        return reportId;
    }

    public void setReportId(UUID reportId) {
        this.reportId = reportId;
    }

    public UUID getJobId() {
        return jobId;
    }

    public void setJobId(UUID jobId) {
        this.jobId = jobId;
    }

    public int getUserId() {
        return userId;
    }

    public void setUserId(int userId) {
        this.userId = userId;
    }

    @Override
    public String toString() {
        return "ReportResult{" + "id=" + id + ", executionTime=" + new Date(executionTime * 1000) + ", reportId=" +
               reportId +
               ", jobId=" + jobId + ", userId='" + userId + '\'' + '}';
    }
}
