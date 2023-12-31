import time
import fnmatch
import os.path
import os
import re
import string

underlines = ['+', '/']

def __section_header(section):
    parts = section.split('/')
    underline = underlines[len(parts) - 1] * len(parts[-1])
    if len(parts) > 0:
        hidden_target = '.. _`label-%s`:' % '-'.join( parts )
        return '\n%s\n%s\n%s\n\n' % (parts[-1], underline, hidden_target )
    else:
        return '\n%s\n%s\n\n' % (parts[-1], underline )


def __section_intro(section):
    parts = section.split('/')
    return '%s.rst' % '-'.join( [x.split(' ')[0] for x in parts] )


def __include_page( output, page ):
    output.write( '.. include:: %s\n' % page )      
    # output.write( '.. raw:: LaTeX\n\n' )
    # output.write( '   \\newpage\n\n')
    
    ref = '/'.join( page.split('.')[0].split('-') )
    if ref.upper() == ref: # macros
        ref = 'BOOST_MPL_%s' % ref
        output.write( 
              ( '.. |%(ref)s| replace:: |``%(ref)s``|__\n'
                + '.. |``%(ref)s``| replace:: :refentry:`%(ref)s`\n'
                + '__ `%(ref)s`_\n' ) 
                    % { 'ref': ref }
            )
    elif ref.lower() == ref:
        output.write( 
              ( '.. |%(ref)s| replace:: |``%(ref)s``|__\n'
                + '.. |``%(ref)s``| replace:: :refentry:`%(ref)s`\n'
                + '__ `%(ref)s`_\n' ) 
                    % { 'ref': ref }
            )
    else:
        if ref.find( '/' ) == -1:
            ref = ' '.join( filter( lambda x: len(x) > 0, re.split( '([A-Z][a-z]+)', ref ) ) )
            output.write( '.. |%(ref)s| replace:: `%(ref)s`_\n' % { 'ref': ref } )

    modtime = time.gmtime( os.stat( page ).st_mtime )    
    output.write( '.. modtime: %s\n' % time.strftime( '%B %d, %Y %H:%M:%S +0000', modtime ) )
    output.write( '\n' )


def __write_index( filename, index ):
    index_file = open( filename, 'w' )
    index.sort()
    for x in index:
        index_file.write( '* |%s|\n' % x )
    
    index_file.close()


def main( filename, dir ):
    sources = filter(
          lambda x: fnmatch.fnmatch(x,"*.rst") and x != filename
        , os.listdir(dir)
        )

    toc = [t.strip() for t in open('%s.toc' % filename).readlines()]
    topics = {}
    for t in toc: topics[t] = []

    concept_index = []
    index = []

    output = open('%s.gen' % filename, 'w')
    output.writelines( open( '%s.rst' % filename, 'r' ).readlines() )
    re_topic = re.compile(r'^..\s+(.+?)//(.+?)(\s*\|\s*(\d+))?\s*$')
    for src in sources:
        placement_spec = open(src, 'r').readline()
        
        topic = 'Unclassified'
        name = None        
        order = -1
        
        match = re_topic.match(placement_spec)
        if match:
            topic = match.group(1)
            name = match.group(2)
            if match.group(3):
                order = int(match.group(4))
        
        if not topics.has_key(topic):
            topics[topic] = []
        
        topics[topic].append((src, order))
        
        if name:
            if topic.find( '/Concepts' ) == -1:
                index.append( name )
            else:
                concept_index.append( name )
        

    for t in toc:
        content = topics[t]
        content.sort( lambda x,y: x[1] - y[1] )
        
        output.write( __section_header(t) )

        intro = __section_intro(t)
        if os.path.exists(intro):
            __include_page( output, intro )
        
        for src in content:
            __include_page( output, src[0] )

    output.close()

    __write_index( 'concepts.gen', concept_index )
    __write_index( 'index.gen', index )
    
    

main( 'refmanual', os.getcwd() )
