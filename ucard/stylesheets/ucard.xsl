<?xml version="1.0" encoding="utf-8"?>
<!-- vim:set noet ts=8 sw=4: -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:u="http://il4p.fr/TR/UCard" version="1.0">
    <xsl:output method="text" />

    <xsl:include href="common.xsl" />

    <xsl:key name="key" match="../../keys/key" use="@name"/>

    <xsl:param name="METADATA_FILNO" select="8" />

    <xsl:template match="/u:applications">
	<xsl:apply-templates select="u:application" />
    </xsl:template>

    <xsl:template match="u:application">
	<xsl:message>  XSL    <xsl:value-of select="@name" />.h</xsl:message>

	<xsl:variable name="header-name">
	    <xsl:call-template name="to-upper">
		<xsl:with-param name="s" select="concat('_', @name, '_H')" />
	    </xsl:call-template>
	</xsl:variable>
	<xsl:variable name="application-name">
	    <xsl:call-template name="to-lower">
		<xsl:with-param name="s" select="@name" />
	    </xsl:call-template>
	</xsl:variable>
	<xsl:document href="{@name}.h" method="text">/*
 * This file is auto-generated.
 * DO NOT EDIT!
 */

#ifndef <xsl:value-of select="$header-name" />
#define <xsl:value-of select="$header-name" />

#include &lt;sys/types.h>

struct ucard_application *<xsl:value-of select="$application-name" />_application_new (password_request_callback on_password_requested);

<xsl:call-template name="generate-files-c-api">
    <xsl:with-param name="headers-only" select="1" />
    <xsl:with-param name="application-name" select="$application-name" />
</xsl:call-template>
#endif /* !<xsl:value-of select="$header-name" /> */
</xsl:document>
	<xsl:message>  XSL    <xsl:value-of select="@name" />.c</xsl:message>
	<xsl:document href="{@name}.c" method="text">/*
 * This file is auto-generated.
 * DO NOT EDIT!
 */

#include &lt;sys/types.h>
#include &lt;stdlib.h>

#include &lt;freefare.h>
#include &lt;ucard.h>

#include "<xsl:value-of select="@name" />.h"

struct ucard_application *
<xsl:value-of select="$application-name" />_application_new (password_request_callback on_password_requested)
{
    struct ucard_application *uapplication = ucard_application_new (on_password_requested);

    if (uapplication) {
	ucard_application_set_aid (uapplication, <xsl:value-of select="@aid" />);
	ucard_application_set_name (uapplication, "<xsl:value-of select="@name" />");
	<xsl:for-each select="u:keys/u:key">uint8_t key<xsl:call-template name="get-key-number" />_data[] = <xsl:call-template name="format-key-data"><xsl:with-param name="key-data" select="@data" /></xsl:call-template>;
	    ucard_application_add_key (uapplication, mifare_desfire_<xsl:call-template name="to-lower"><xsl:with-param name="s" select="@type" /></xsl:call-template>_key_new (key<xsl:call-template name="get-key-number" />_data));
	<xsl:if test="position() = 1">ucard_application_add_key (uapplication, NULL); /* User read key */
	</xsl:if>
	</xsl:for-each>
	<xsl:for-each select="u:files/u:file">
	    <xsl:variable name="file-name">
		<xsl:call-template name="to-lower">
		    <xsl:with-param name="s" select="@name" />
		</xsl:call-template>
		</xsl:variable>
		<xsl:variable name="access-rights">
		    <xsl:text>0x</xsl:text>
		    <!-- read -->
		    <xsl:variable name="read-key-no">
			<xsl:choose>
			    <xsl:when test="@read-key">
				<xsl:call-template name="key-no-by-key-name">
				    <xsl:with-param name="key-name" select="@read-key" />
				</xsl:call-template>
			    </xsl:when>
			    <xsl:otherwise>
				<xsl:text>1</xsl:text>
			    </xsl:otherwise>
			</xsl:choose>
		    </xsl:variable>
		    <xsl:value-of select="substring ($hex, $read-key-no + 1, 1)" />
		    <!-- write -->
		    <xsl:variable name="write-key-no">
			<xsl:call-template name="key-no-by-key-name">
			    <xsl:with-param name="key-name" select="@write-key" />
			</xsl:call-template>
		    </xsl:variable>
		    <xsl:value-of select="substring ($hex, $write-key-no + 1, 1)" />
		    <!-- read-write -->
		    <xsl:variable name="read-write-key-no">
			<xsl:call-template name="key-no-by-key-name">
			    <xsl:with-param name="key-name" select="@read-write-key" />
			</xsl:call-template>
		    </xsl:variable>
		    <xsl:value-of select="substring ($hex, $read-write-key-no + 1, 1)" />
		    <!-- change-settings -->
		    <xsl:variable name="change-settings-key-no">
			<xsl:call-template name="key-no-by-key-name">
			    <xsl:with-param name="key-name" select="@change-settings-key" />
			</xsl:call-template>
		    </xsl:variable>
		    <xsl:value-of select="substring ($hex, $change-settings-key-no + 1, 1)" />
		</xsl:variable>
		<xsl:choose>
		    <xsl:when test="@type = 'std-data-file'">ucard_application_add_std_data_file (uapplication, "<xsl:value-of select="@name" />", <xsl:value-of select="$access-rights" />, <xsl:value-of select="u:file-size + 1" />);
	</xsl:when>
		    <xsl:when test="@type = 'backup-data-file'">ucard_application_add_backup_data_file (uapplication, "<xsl:value-of select="@name" />", <xsl:value-of select="$access-rights" />, <xsl:value-of select="u:file-size + 1" />);
	</xsl:when>
		    <xsl:when test="@type = 'value-file'">ucard_application_add_value_file (uapplication, "<xsl:value-of select="@name" />", <xsl:value-of select="$access-rights" />, <xsl:choose>
			    <xsl:when test="u:lower-limit">
				<xsl:value-of select="u:lower-limit" />
			    </xsl:when>
			    <xsl:otherwise>
				<xsl:text>0</xsl:text>
			    </xsl:otherwise>
			    </xsl:choose>, <xsl:choose>
			    <xsl:when test="u:upper-limit">
				<xsl:value-of select="u:upper-limit" />
			    </xsl:when>
			    <xsl:otherwise>
				<xsl:text>0</xsl:text>
			    </xsl:otherwise>
			    </xsl:choose>, <xsl:choose>
			    <xsl:when test="u:value">
				<xsl:value-of select="u:value" />
			    </xsl:when>
			    <xsl:otherwise>
				<xsl:text>0</xsl:text>
			    </xsl:otherwise>
			    </xsl:choose>, <xsl:choose>
			    <xsl:when test="u:limited-credit-enabled">
				<xsl:text>1</xsl:text>
			    </xsl:when>
			    <xsl:otherwise>
				<xsl:text>0</xsl:text>
			    </xsl:otherwise>
			</xsl:choose><xsl:text>);
	</xsl:text>
		    </xsl:when>
		    <xsl:when test="@type = 'linear-record-file'">ucard_application_add_linear_record_file (uapplication, "<xsl:value-of select="@name" />", <xsl:value-of select="$access-rights" />, <xsl:value-of select="u:record-size" />, <xsl:value-of select="u:record-count" />);
	</xsl:when>
		    <xsl:when test="@type = 'cyclic-record-file'">ucard_application_add_cyclic_record_file (uapplication, "<xsl:value-of select="@name" />", <xsl:value-of select="$access-rights" />, <xsl:value-of select="u:record-size" />, <xsl:value-of select="u:record-count" />);
    </xsl:when>
		</xsl:choose>
	    </xsl:for-each>}

    return uapplication;
}
<xsl:call-template name="generate-files-c-api">
    <xsl:with-param name="application-name" select="$application-name" />
</xsl:call-template>
</xsl:document>
</xsl:template>

<xsl:template name="generate-files-c-api">
    <xsl:param name="headers-only" select="0" />
    <xsl:param name="application-name" />
<xsl:for-each select="u:files/u:file">
    <xsl:variable name="file-name">
	<xsl:call-template name="to-lower">
	    <xsl:with-param name="s" select="@name" />
	</xsl:call-template>
    </xsl:variable>
    <xsl:variable name="file-no">
	<xsl:choose>
	    <xsl:when test="position() - 1 &lt; $METADATA_FILNO">
		<xsl:value-of select="position() - 1" />
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:value-of select="position()" />
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:variable>
    <xsl:variable name="read-key-no">
	<xsl:choose>
	    <xsl:when test="@read-key">
		<xsl:call-template name="key-no-by-key-name">
		    <xsl:with-param name="key-name" select="@read-key" />
		</xsl:call-template>
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:text>1</xsl:text>
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:variable>
    <xsl:variable name="write-key-no">
	<xsl:call-template name="key-no-by-key-name">
	    <xsl:with-param name="key-name" select="@write-key" />
	</xsl:call-template>
    </xsl:variable>
    <xsl:variable name="read-write-key-no">
	<xsl:call-template name="key-no-by-key-name">
	    <xsl:with-param name="key-name" select="@read-write-key" />
	</xsl:call-template>
    </xsl:variable>
    <xsl:variable name="change-settings-key-no">
	<xsl:call-template name="key-no-by-key-name">
	    <xsl:with-param name="key-name" select="@change-settings-key" />
	</xsl:call-template>
    </xsl:variable>

    <!-- We will try to use the read-write key if available -->
    <xsl:variable name="actual-read-key-no">
	<xsl:choose>
	    <xsl:when test="not($read-write-key-no = $MDAR_DENY)">
		<xsl:value-of select="$read-write-key-no" />
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:value-of select="$read-key-no" />
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:variable>
    <xsl:variable name="actual-write-key-no">
	<xsl:choose>
	    <xsl:when test="not($read-write-key-no = $MDAR_DENY)">
		<xsl:value-of select="$read-write-key-no" />
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:value-of select="$write-key-no" />
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:variable>

    <xsl:choose>
	<xsl:when test="@type = 'std-data-file' or @type = 'backup-data-file'">
	    <xsl:if test="not($actual-read-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_read_data (struct ucard *ucard, const struct ucard_application *uapplication, const uint32_t offset, const uint32_t length, void *data)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_read_data (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$actual-read-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$actual-read-key-no" />), offset, length, data);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	    <xsl:if test="not($actual-write-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_write_data (struct ucard *ucard, const struct ucard_application *uapplication, const uint32_t offset, const uint32_t length, void *data)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_write_data (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$actual-write-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$actual-write-key-no" />), offset, length, data);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	</xsl:when>
	<xsl:when test="@type = 'value-file'">
	    <!-- A value file has inhabitual permissions -->
	    <xsl:variable name="any-valid-key-no">
		<xsl:choose>
		    <xsl:when test="not($actual-write-key-no = $MDAR_DENY)">
			<xsl:value-of select="$actual-write-key-no" />
		    </xsl:when>
		    <xsl:otherwise>
			<xsl:value-of select="$actual-read-key-no" />
		    </xsl:otherwise>
		</xsl:choose>
	    </xsl:variable>
	    <xsl:if test="not($read-write-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_credit (struct ucard *ucard, const struct ucard_application *uapplication, const int32_t amount)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_value_file_credit (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$read-write-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$read-write-key-no" />), amount);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	    <xsl:if test="not($any-valid-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_debit (struct ucard *ucard, const struct ucard_application *uapplication, const int32_t amount)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_value_file_debit (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$any-valid-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$any-valid-key-no" />), amount);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>

	    <xsl:if test="not($any-valid-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_limited_credit (struct ucard *ucard, const struct ucard_application *uapplication, const int32_t amount)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_value_file_limited_credit (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$any-valid-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$any-valid-key-no" />), amount);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>

	    <xsl:if test="not($any-valid-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_get_value (struct ucard *ucard, const struct ucard_application *uapplication, int32_t *amount)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_value_file_get_value (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$any-valid-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$any-valid-key-no" />), amount);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	</xsl:when>
	<xsl:when test="@type = 'linear-record-file' or @type = 'cyclic-record-file'">
	    <xsl:if test="not($actual-read-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="return-type">int</xsl:with-param>
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_read_records (struct ucard *ucard, const struct ucard_application *uapplication, const uint32_t offset, const uint32_t length, void *data)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_read_records (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$actual-read-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$actual-read-key-no" />), offset, length, data);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	    <xsl:if test="not($actual-write-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_write_record (struct ucard *ucard, const struct ucard_application *uapplication, const uint32_t offset, const uint32_t length, void *data)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_write_record (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$actual-write-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$actual-write-key-no" />), offset, length, data);</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	    <xsl:if test="not($read-write-key-no = $MDAR_DENY)">
		<xsl:call-template name="c-api-function">
		    <xsl:with-param name="header-only" select="$headers-only" />
		    <xsl:with-param name="function-prototype"><xsl:value-of select="$application-name" />_<xsl:value-of select="$file-name" />_clear (struct ucard *ucard, const struct ucard_application *uapplication)</xsl:with-param>
		    <xsl:with-param name="function-body">
    return ucard_clear_record_file (ucard, uapplication, <xsl:value-of select="$file-no" />, <xsl:value-of select="$read-write-key-no" />, ucard_application_get_key (uapplication, <xsl:value-of select="$read-write-key-no" />));</xsl:with-param>
		</xsl:call-template>
	    </xsl:if>
	</xsl:when>
    </xsl:choose>
</xsl:for-each>
    </xsl:template>

    <xsl:template name="c-api-function">
	<xsl:param name="return-type" select="'int'" />
	<xsl:param name="function-prototype" />
	<xsl:param name="function-body" />
	<xsl:param name="header-only" />

	<xsl:if test="not($header-only)"><xsl:text>
</xsl:text></xsl:if>
	<xsl:value-of select="$return-type" />
	<xsl:choose>
	    <xsl:when test="$header-only">
		<xsl:text>    </xsl:text>
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:text>
</xsl:text>
	    </xsl:otherwise>
	</xsl:choose>
	<xsl:value-of select="$function-prototype" />
	<xsl:choose>
	    <xsl:when test="$header-only">
		<xsl:text>;
</xsl:text>
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:text> {</xsl:text>
		<xsl:value-of select="$function-body" />
		<xsl:text>
}
</xsl:text>
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:template>

</xsl:stylesheet>
