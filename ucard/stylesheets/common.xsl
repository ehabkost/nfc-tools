<?xml version="1.0" encoding="utf-8"?>
<!-- vim:set noet ts=8 sw=4: -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:u="http://il4p.fr/TR/UCard" version="1.0">

    <xsl:variable name="hex">0123456789ABCDEF</xsl:variable>

    <xsl:variable name="MDAR_DENY">15</xsl:variable>

    <xsl:template name="format-key-data">
	<xsl:param name="key-data" />
	<xsl:choose>
	    <xsl:when test="starts-with($key-data, '0x')">
		<xsl:text>{</xsl:text>
		<xsl:call-template name="format-hex">
		    <xsl:with-param name="stream" select="substring ($key-data, 3)" />
		</xsl:call-template>
		<xsl:text> }</xsl:text>
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:value-of select="$key-data" />
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:template>

    <xsl:template name="format-hex">
	<xsl:param name="stream" />
	<xsl:text> 0x</xsl:text>
	<xsl:value-of select="substring ($stream, 1, 2)" />
	<xsl:if test="string-length ($stream) > 2">
	    <xsl:text>,</xsl:text>
	    <xsl:call-template name="format-hex">
		<xsl:with-param name="stream" select="substring ($stream, 3)" />
	    </xsl:call-template>
	</xsl:if>
    </xsl:template>

    <xsl:template name="to-lower">
	<xsl:param name="s" />
	<xsl:value-of select="translate ($s, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ-.', 'abcdefghijklmnopqrstuvwxyz__')" />
    </xsl:template>
    
    <xsl:template name="to-upper">
	<xsl:param name="s" />
	<xsl:value-of select="translate ($s, 'abcdefghijklmnopqrstuvwxyz-.', 'ABCDEFGHIJKLMNOPQRSTUVWXYZ__')" />
    </xsl:template>

    <xsl:template name="get-key-number">
	<xsl:choose>
	    <xsl:when test="position() = 1">
		<xsl:text>0</xsl:text>
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:value-of select="position()" />
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:template>

    <xsl:template name="key-no-by-key-name">
	<xsl:param name="key-name" />
	<xsl:param name="default-key-no" select="'15'" />

	<xsl:choose>
	    <xsl:when test="not($key-name) or $key-name = 'none'">
		<xsl:value-of select="$default-key-no" />
	    </xsl:when>
	    <xsl:when test="$key-name = 'all' or $key-name = 'free'">
		<xsl:text>14</xsl:text>
	    </xsl:when>
	    <xsl:otherwise>
		<xsl:if test="not(../../u:keys/u:key[@name = $key-name])">
		    <xsl:message terminate="yes">No key '<xsl:value-of select="$key-name" />' defined for '<xsl:value-of select="../../@name" />::<xsl:value-of select="@name" />'.</xsl:message>
		</xsl:if>
		<xsl:for-each select="../../u:keys/u:key[@name = $key-name]">
		    <xsl:variable name="v">
			<xsl:value-of select="count (preceding-sibling::u:key)" />
		    </xsl:variable>
		    <xsl:choose>
			<xsl:when test="$v = 0">
			    <xsl:text>0</xsl:text>
			</xsl:when>
			<xsl:otherwise>
			    <xsl:value-of select="$v + 1" />
			</xsl:otherwise>
		    </xsl:choose>
		</xsl:for-each>
	    </xsl:otherwise>
	</xsl:choose>
    </xsl:template>
</xsl:stylesheet>
