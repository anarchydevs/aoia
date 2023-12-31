<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">
  <xsl:include href="http://docbook.sourceforge.net/release/xsl/current/html/chunker.xsl"/>
  <xsl:include href="http://docbook.sourceforge.net/release/xsl/current/common/common.xsl"/>

  <!-- The root of the Boost directory -->
  <xsl:param name="boost.root"/>

  <!-- The number of columns in a source file line -->
  <xsl:param name="boost.source.columns" select="78"/>

  <xsl:output method="text"/>
  <xsl:template match="/">
    <xsl:if test="$boost.root">
      <!-- Output testsuite Jamfiles -->
      <xsl:apply-templates select="//testsuite" mode="Jamfile"/>

      <!-- Output any source files that are written in XML -->
      <xsl:apply-templates select="//source" mode="testsuite.generate"/>
    </xsl:if>

    <xsl:if test="not($boost.root)">
      <xsl:message>
        Please set the XSL stylesheet parameter "boost.root" to the top-level
        Boost directory (i.e., BOOST_ROOT)
      </xsl:message>
    </xsl:if>
  </xsl:template>

  <xsl:template match="testsuite" mode="Jamfile">
    <xsl:variable name="subproject"
      select="concat('libs/',ancestor::library/attribute::dirname,'/test')"/>
    <xsl:variable name="filename" 
      select="concat($boost.root,'/libs/',ancestor::library/attribute::dirname,
                     '/test/Jamfile')"/>
    <xsl:call-template name="write.text.chunk">
      <xsl:with-param name="filename" select="$filename"/>
      <xsl:with-param name="content">
        <xsl:call-template name="source.print.header">
          <xsl:with-param name="prefix" select="'#'"/>
        </xsl:call-template>

# Testing Jamfile autogenerated from XML source
subproject <xsl:value-of select="$subproject"/> ;

# bring in rules for testing
SEARCH on testing.jam = $(BOOST_BUILD_PATH) ;
include testing.jam ;

# Make tests run by default.
DEPENDS all : test ;

{
  # look in BOOST_ROOT for sources first, just in this Jamfile
  local SEARCH_SOURCE = $(BOOST_ROOT) $(SEARCH_SOURCE) ;

  test-suite <xsl:value-of select="ancestor::library/attribute::dirname"/>
    : <xsl:apply-templates mode="Jamfile"/> ;
}
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="compile-test|link-test|run-test|compile-fail-test|link-fail-test|run-fail-test"
    mode="Jamfile">
    <xsl:variable name="fullname"
      select="concat(substring-before(local-name(.), '-test'), ' libs/', 
                     ancestor::library/attribute::dirname, '/test/', 
                     @filename)"/>
    <xsl:text>[ </xsl:text>
    <xsl:value-of select="$fullname"/>
    <xsl:apply-templates select="lib" mode="Jamfile"/>
    <xsl:text> : </xsl:text>
    <!-- ... -->
    <xsl:text> : </xsl:text>
    <!-- ... -->
    <xsl:text> : </xsl:text>
    <xsl:apply-templates select="requirement" mode="Jamfile"/>
    <!-- ... -->
    <xsl:text> : </xsl:text>
    <xsl:if test="@name">
      <xsl:value-of select="@name"/>
    </xsl:if>
    <xsl:text> ]</xsl:text>
  </xsl:template>

  <xsl:template match="lib" mode="Jamfile">
    <xsl:text> &lt;lib&gt;</xsl:text><xsl:value-of select="text()"/>
  </xsl:template>

  <xsl:template match="requirement" mode="Jamfile">
    <xsl:if test="count(preceding-sibling::requirement) &gt; 0">
      <xsl:text> </xsl:text>
    </xsl:if>
    <xsl:value-of select="concat('&lt;', @name, '&gt;')"/>
    <xsl:value-of select="."/>
  </xsl:template>

  <xsl:template match="source" mode="testsuite.generate">
    <xsl:variable name="filename" 
      select="concat($boost.root,'/libs/',ancestor::library/attribute::dirname,
                     '/test/',../@filename)"/>

    <xsl:variable name="prefix" select="'//'"/>

    <xsl:call-template name="write.text.chunk">
      <xsl:with-param name="filename" select="$filename"/>
      <xsl:with-param name="content">
        <xsl:call-template name="source.print.header">
          <xsl:with-param name="prefix" select="'//'"/>
        </xsl:call-template>
        <!-- Source code -->
        <xsl:apply-templates mode="testsuite.generate"/>
      </xsl:with-param>
    </xsl:call-template>
  </xsl:template>

  <xsl:template match="snippet" mode="testsuite.generate">
    <xsl:variable name="snippet-name" select="@name"/>
    <xsl:value-of select="//programlisting[@name=$snippet-name]"/>
  </xsl:template>

  <!-- Prints a header to a source file containing the library name,
       copyright information, license information, and a link to the Boost
       web site. The text is output using the "prefix" parameter at the
       beginning of each line (it should denote a single-line comment) and
       will only print up to boost.source.columns characters per line
       (including the prefix). -->
  <xsl:template name="source.print.header">
    <xsl:param name="prefix"/>

    <!-- Library name -->
    <xsl:value-of select="concat($prefix, ' ', 
                                 ancestor::library/attribute::name, 
                                 ' library')"/>
    <xsl:text>&#10;</xsl:text>
    
    <!-- Copyrights -->
    <xsl:apply-templates select="ancestor::library/libraryinfo/copyright"
      mode="testsuite.generate">
      <xsl:with-param name="prefix" select="$prefix"/>
    </xsl:apply-templates>
        
    <!-- Legal notice -->
    <xsl:apply-templates select="ancestor::library/libraryinfo/legalnotice"
      mode="testsuite.generate">
      <xsl:with-param name="prefix" select="$prefix"/>
    </xsl:apply-templates>
    
    <xsl:text>&#10;&#10;</xsl:text>
    
    <!-- For more information... -->
    <xsl:value-of 
      select="concat($prefix,
              ' For more information, see http://www.boost.org/')"/>
    <xsl:text>&#10;</xsl:text>
  </xsl:template>

  <xsl:template match="copyright" mode="testsuite.generate">
    <xsl:param name="prefix"/>

    <xsl:text>&#10;</xsl:text>

    <xsl:value-of select="concat($prefix, ' Copyright (C) ')"/>
    <xsl:call-template name="copyright.years">
      <xsl:with-param name="years" select="year"/>
      <xsl:with-param name="print.ranges" select="1"/>
      <xsl:with-param name="single.year.ranges" select="1"/>
    </xsl:call-template>

    <xsl:text> </xsl:text>

    <xsl:apply-templates select="holder" mode="titlepage.mode"/>
  </xsl:template>

  <xsl:template match="legalnotice" mode="testsuite.generate">
    <xsl:param name="prefix"/>

    <xsl:variable name="text" select="normalize-space(.)"/>

    <xsl:text>&#10;&#10;</xsl:text>
    <xsl:value-of select="concat($prefix, ' ')"/>

    <xsl:call-template name="source.print.legalnotice">
      <xsl:with-param name="prefix" select="$prefix"/>
      <xsl:with-param name="text" select="$text"/>
      <xsl:with-param name="column" select="string-length($prefix) + 1"/>
    </xsl:call-template>
  </xsl:template>

  <xsl:template name="source.print.legalnotice">
    <xsl:param name="prefix"/>
    <xsl:param name="text"/>
    <xsl:param name="column"/>

    <xsl:if test="contains($text, ' ') or string-length($text) &gt; 0">
      <xsl:variable name="word">
        <xsl:choose>
          <xsl:when test="contains($text, ' ')">
            <xsl:value-of select="substring-before($text, ' ')"/>
          </xsl:when>
          <xsl:otherwise>
            <xsl:value-of select="$text"/>
          </xsl:otherwise>
        </xsl:choose>
      </xsl:variable>

      <xsl:variable name="rest" select="substring-after($text, ' ')"/>

      <xsl:choose>
        <xsl:when 
          test="$column + string-length($word) &gt; $boost.source.columns">
          <xsl:text>&#10;</xsl:text>
          <xsl:value-of select="concat($prefix, ' ')"/>
          <xsl:call-template name="source.print.legalnotice">
            <xsl:with-param name="prefix" select="$prefix"/>
            <xsl:with-param name="text" select="$text"/>
            <xsl:with-param name="column" select="string-length($prefix) + 1"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="concat($word, ' ')"/>
          <xsl:call-template name="source.print.legalnotice">
            <xsl:with-param name="prefix" select="$prefix"/>
            <xsl:with-param name="text" select="$rest"/>
            <xsl:with-param name="column" 
              select="$column + string-length($word) + 1"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:if>
  </xsl:template>

  <xsl:template match="*" mode="testsuite.generate"/>
</xsl:stylesheet>
