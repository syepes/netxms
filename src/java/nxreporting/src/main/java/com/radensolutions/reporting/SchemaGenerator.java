package com.radensolutions.reporting;

import com.radensolutions.reporting.model.Notification;
import com.radensolutions.reporting.model.ReportResult;
import org.hibernate.boot.MetadataSources;
import org.hibernate.boot.registry.StandardServiceRegistryBuilder;
import org.hibernate.boot.spi.MetadataImplementor;
import org.hibernate.tool.hbm2ddl.SchemaExport;

public class SchemaGenerator {
    /**
     * Generate schema files for all supported databases
     */
    public void generateSchemas() {
        generateSchema("postgres", "org.hibernate.dialect.PostgreSQL9Dialect");
        generateSchema("mysql", "org.hibernate.dialect.MySQLDialect");
        generateSchema("mssql", "org.hibernate.dialect.SQLServerDialect");
        generateSchema("oracle", "org.hibernate.dialect.Oracle10gDialect");
        generateSchema("informix", "org.hibernate.dialect.InformixDialect");
    }

    private void generateSchema(String name, String dialect) {
        MetadataSources metadata = new MetadataSources(
                new StandardServiceRegistryBuilder().applySetting("hibernate.dialect", dialect).build());

        metadata.addAnnotatedClass(ReportResult.class);
        metadata.addAnnotatedClass(Notification.class);
        SchemaExport export = new SchemaExport((MetadataImplementor) metadata.buildMetadata());
        export.setOutputFile("sql/" + name + "/nxreporting.sql");
        export.setFormat(true);
        export.execute(true, false, false, false);
    }
}
