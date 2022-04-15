/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */

#define LOGGER_ERROR

#include <stdio.h>
#include <xercesc/util/IOException.hpp>
#include <xercesc/util/RefStackOf.hpp>
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/Janitor.hpp>
#include <xercesc/sax2/ContentHandler.hpp>
#include <xercesc/sax2/LexicalHandler.hpp>
#include <xercesc/sax2/DeclHandler.hpp>

#include "XMLReaderFactory.hpp"

#include <xercesc/sax/DTDHandler.hpp>
#include <xercesc/sax/ErrorHandler.hpp>
#include <xercesc/sax/EntityResolver.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include <xercesc/sax/SAXException.hpp>
#include <xercesc/internal/XMLScannerResolver.hpp>

#include "SAX2XMLReaderImpl.hpp"

#include <xercesc/validators/common/GrammarResolver.hpp>
#include <xercesc/framework/XMLGrammarPool.hpp>
#include <xercesc/framework/XMLSchemaDescription.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/util/XMLEntityResolver.hpp>
#include <string.h>
#include <stdio.h>
#include <iostream>


#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include "fmt/core.h"


#include "logging.h"

static spdlog::logger logger = getLog();


xercesc_3_2::SAX2XMLReader * XMLReaderFactoryLoc::createXMLReaderLoc(  xercesc_3_2::MemoryManager* const  manager
                                                  , xercesc_3_2::XMLGrammarPool* const gramPool)
{
    printf("‰s","\n\n\n\nthis one this one\n\n\n\n");
    std::cout<<"how come it doesn't appear?"<<std::endl;
    logger.warn("Can you see me?");
	//exit(-1);
    xercesc_3_2::SAX2XMLReaderImplLoc* pImpl=new (manager) xercesc_3_2::SAX2XMLReaderImplLoc(manager, gramPool);
    return pImpl;
}

XERCES_CPP_NAMESPACE_BEGIN

const XMLCh gDTDEntityStr[] =
{
    chOpenSquare, chLatin_d, chLatin_t, chLatin_d, chCloseSquare, chNull
};



typedef JanitorMemFunCall<SAX2XMLReaderImplLoc>    CleanupType;
typedef JanitorMemFunCall<SAX2XMLReaderImplLoc>    ResetInProgressType;


SAX2XMLReaderImplLoc::SAX2XMLReaderImplLoc(xercesc_3_2::MemoryManager* const  manager
                                   , xercesc_3_2::XMLGrammarPool* const gramPool):

    fNamespacePrefix(false)
    , fAutoValidation(false)
    , fValidation(false)
    , fParseInProgress(false)
    , fHasExternalSubset(false)
    , fElemDepth(0)
    , fAdvDHCount(0)
    , fAdvDHListSize(32)
    , fDocHandler(0)
    , fTempAttrVec(0)
    , fPrefixesStorage(0)
    , fPrefixes(0)
    , fPrefixCounts(0)
    , fTempQName(0)
    , fDTDHandler(0)
    , fEntityResolver(0)
    , fXMLEntityResolver(0)
    , fErrorHandler(0)
    , fPSVIHandler(0)
    , fLexicalHandler(0)
    , fDeclHandler(0)
    , fAdvDHList(0)
    , fScanner(0)
    , fGrammarResolver(0)
    , fURIStringPool(0)
    , fValidator(0)
    , fMemoryManager(manager)
    , fGrammarPool(gramPool)
{
    CleanupType cleanup(this, &SAX2XMLReaderImplLoc::cleanUp);

    try
    {
        initialize();
    }
    catch(const OutOfMemoryException&)
    {
        // Don't cleanup when out of memory, since executing the
        // code can cause problems.
        cleanup.release();

        throw;
    }

    cleanup.release();
}

SAX2XMLReaderImplLoc::~SAX2XMLReaderImplLoc()
{
    cleanUp();
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Initialize/Cleanup methods
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::initialize()
{
    // Create grammar resolver and string pool that we pass to the scanner
    fGrammarResolver = new (fMemoryManager) GrammarResolver(fGrammarPool, fMemoryManager);
    fURIStringPool = fGrammarResolver->getStringPool();

    //  Create a scanner and tell it what validator to use. Then set us
    //  as the document event handler so we can fill the DOM document.
    fScanner = XMLScannerResolver::getDefaultScanner(0, fGrammarResolver, fMemoryManager);
    fScanner->setURIStringPool(fURIStringPool);

    // Create the initial advanced handler list array and zero it out
    fAdvDHList = (XMLDocumentHandler**) fMemoryManager->allocate
    (
        fAdvDHListSize * sizeof(XMLDocumentHandler*)
    );//new XMLDocumentHandler*[fAdvDHListSize];
    memset(fAdvDHList, 0, sizeof(void*) * fAdvDHListSize);

    // SAX2 default is for namespaces (feature http://xml.org/sax/features/namespaces) to be on
    setDoNamespaces(true) ;

    // default: schema is on
    setDoSchema(true);

    fPrefixesStorage = new (fMemoryManager) XMLStringPool(109, fMemoryManager) ;
    fPrefixes        = new (fMemoryManager) ValueStackOf<unsigned int> (30, fMemoryManager) ;
    fTempAttrVec     = new (fMemoryManager) RefVectorOf<XMLAttr>  (10, false, fMemoryManager) ;
    fPrefixCounts    = new (fMemoryManager) ValueStackOf<XMLSize_t>(10, fMemoryManager) ;
    fTempQName       = new (fMemoryManager) XMLBuffer(32, fMemoryManager);
}


void SAX2XMLReaderImplLoc::cleanUp()
{
    fMemoryManager->deallocate(fAdvDHList);//delete [] fAdvDHList;
    delete fScanner;
    delete fPrefixesStorage;
    delete fPrefixes;
    delete fTempAttrVec;
    delete fPrefixCounts;
    delete fGrammarResolver;
    delete fTempQName;
    // grammar pool must do this
    //delete fURIStringPool;
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Advanced document handler list maintenance methods
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::installAdvDocHandler(XMLDocumentHandler* const toInstall)
{
    // See if we need to expand and do so now if needed
    if (fAdvDHCount == fAdvDHListSize)
    {
        // Calc a new size and allocate the new temp buffer
        const XMLSize_t newSize = (XMLSize_t)(fAdvDHListSize * 1.5);
        XMLDocumentHandler** newList = (XMLDocumentHandler**) fMemoryManager->allocate
        (
            newSize * sizeof(XMLDocumentHandler*)
        );//new XMLDocumentHandler*[newSize];

        // Copy over the old data to the new list and zero out the rest
        memcpy(newList, fAdvDHList, sizeof(void*) * fAdvDHListSize);
        memset
        (
            &newList[fAdvDHListSize]
            , 0
            , sizeof(void*) * (newSize - fAdvDHListSize)
        );

        // And now clean up the old array and store the new stuff
        fMemoryManager->deallocate(fAdvDHList);//delete [] fAdvDHList;
        fAdvDHList = newList;
        fAdvDHListSize = newSize;
    }

    // Add this new guy into the empty slot
    fAdvDHList[fAdvDHCount++] = toInstall;

    //
    //  Install ourself as the document handler with the scanner. We might
    //  already be, but its not worth checking, just do it.
    //
    fScanner->setDocHandler(this);
}


bool SAX2XMLReaderImplLoc::removeAdvDocHandler(XMLDocumentHandler* const toRemove)
{
    // If our count is zero, can't be any installed
    if (!fAdvDHCount)
        return false;

    //
    //  Search the array until we find this handler. If we find a null entry
    //  first, we can stop there before the list is kept contiguous.
    //
    XMLSize_t index;
    for (index = 0; index < fAdvDHCount; index++)
    {
        //
        //  We found it. We have to keep the list contiguous, so we have to
        //  copy down any used elements after this one.
        //
        if (fAdvDHList[index] == toRemove)
        {
            //
            //  Optimize if only one entry (pretty common). Otherwise, we
            //  have to copy them down to compact them.
            //
            if (fAdvDHCount > 1)
            {
                index++;
                while (index < fAdvDHCount)
                    fAdvDHList[index - 1] = fAdvDHList[index];
            }

            // Bump down the count and zero out the last one
            fAdvDHCount--;
            fAdvDHList[fAdvDHCount] = 0;

            //
            //  If this leaves us with no advanced handlers and there is
            //  no SAX doc handler installed on us, then remove us from the
            //  scanner as the document handler.
            //
            if (!fAdvDHCount && !fDocHandler)
                fScanner->setDocHandler(0);

            return true;
        }
    }

    // Never found it
    return false;
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc Validator functions
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::setValidator(XMLValidator* valueToAdopt)
{
    fValidator = valueToAdopt;
    fScanner->setValidator(valueToAdopt);
}

XMLValidator* SAX2XMLReaderImplLoc::getValidator() const
{
    return fScanner->getValidator();
}

// ---------------------------------------------------------------------------
//  SAX2XMLReader Interface
// ---------------------------------------------------------------------------
XMLSize_t SAX2XMLReaderImplLoc::getErrorCount() const
{
    return fScanner->getErrorCount();
}

void SAX2XMLReaderImplLoc::setContentHandler(ContentHandler* const handler)
{
    fDocHandler = handler;
    if (fDocHandler)
    {
        //
        //  Make sure we are set as the document handler with the scanner.
        //  We may already be (if advanced handlers are installed), but its
        //  not worthing checking, just do it.
        //
        fScanner->setDocHandler(this);
    }
     else
    {
        //
        //  If we don't have any advanced handlers either, then deinstall us
        //  from the scanner because we don't need document events anymore.
        //
        if (!fAdvDHCount)
            fScanner->setDocHandler(0);
    }

}

void SAX2XMLReaderImplLoc::setDTDHandler(DTDHandler* const handler)
{
    fDTDHandler = handler;
    if (fDTDHandler)
        fScanner->setDocTypeHandler(this);
    else
        fScanner->setDocTypeHandler(0);
}


void SAX2XMLReaderImplLoc::setErrorHandler(ErrorHandler* const handler)
{
    //
    //  Store the handler. Then either install or deinstall us as the
    //  error reporter on the scanner.
    //
    fErrorHandler = handler;
    if (fErrorHandler) {
        fScanner->setErrorReporter(this);
        fScanner->setErrorHandler(fErrorHandler);
    }
    else {
        fScanner->setErrorReporter(0);
        fScanner->setErrorHandler(0);
    }
}

void SAX2XMLReaderImplLoc::setPSVIHandler(PSVIHandler* const handler)
{
    fPSVIHandler = handler;
    if (fPSVIHandler) {
        fScanner->setPSVIHandler(fPSVIHandler);
    }
    else {
        fScanner->setPSVIHandler(0);
    }
}

void SAX2XMLReaderImplLoc::setLexicalHandler(LexicalHandler* const handler)
{
    fLexicalHandler = handler;
    if (fLexicalHandler)
        fScanner->setDocTypeHandler(this);
    else
        fScanner->setDocTypeHandler(0);
}

void SAX2XMLReaderImplLoc::setDeclarationHandler(DeclHandler* const handler)
{
    fDeclHandler = handler;
    if (fDeclHandler)
        fScanner->setDocTypeHandler(this);
    else
        fScanner->setDocTypeHandler(0);
}


void SAX2XMLReaderImplLoc::setEntityResolver(EntityResolver* const resolver)
{
    fEntityResolver = resolver;
    if (fEntityResolver) {
        fScanner->setEntityHandler(this);
        fXMLEntityResolver = 0;
    }
    else {
        fScanner->setEntityHandler(0);
    }
}

void SAX2XMLReaderImplLoc::setXMLEntityResolver(XMLEntityResolver* const resolver)
{
    fXMLEntityResolver = resolver;
    if (fXMLEntityResolver) {
        fScanner->setEntityHandler(this);
        fEntityResolver = 0;
    }
    else {
        fScanner->setEntityHandler(0);
    }
}

void SAX2XMLReaderImplLoc::setExitOnFirstFatalError(const bool newState)
{
    fScanner->setExitOnFirstFatal(newState);
}

void SAX2XMLReaderImplLoc::setValidationConstraintFatal(const bool newState)
{
    fScanner->setValidationConstraintFatal(newState);
}

void SAX2XMLReaderImplLoc::parse (const   InputSource&    source)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAX2XMLReaderImplLoc::resetInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(source);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }
}

void SAX2XMLReaderImplLoc::parse (const   XMLCh* const    systemId)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAX2XMLReaderImplLoc::resetInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(systemId);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }
}

void SAX2XMLReaderImplLoc::parse (const   char* const     systemId)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAX2XMLReaderImplLoc::resetInProgress);

    try
    {
        fParseInProgress = true;
        fScanner->scanDocument(systemId);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Progressive parse methods
// ---------------------------------------------------------------------------
bool SAX2XMLReaderImplLoc::parseFirst( const   XMLCh* const    systemId
                            ,       XMLPScanToken&  toFill)
{
    //
    //  Avoid multiple entrance. We cannot enter here while a regular parse
    //  is in progress.
    //
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    return fScanner->scanFirst(systemId, toFill);
}

bool SAX2XMLReaderImplLoc::parseFirst( const   char* const     systemId
                            ,       XMLPScanToken&  toFill)
{
    //
    //  Avoid multiple entrance. We cannot enter here while a regular parse
    //  is in progress.
    //
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    return fScanner->scanFirst(systemId, toFill);
}

bool SAX2XMLReaderImplLoc::parseFirst( const   InputSource&    source
                            ,       XMLPScanToken&  toFill)
{
    //
    //  Avoid multiple entrance. We cannot enter here while a regular parse
    //  is in progress.
    //
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    return fScanner->scanFirst(source, toFill);
}

bool SAX2XMLReaderImplLoc::parseNext(XMLPScanToken& token)
{
    return fScanner->scanNext(token);
}

void SAX2XMLReaderImplLoc::parseReset(XMLPScanToken& token)
{
    // Reset the scanner
    fScanner->scanReset(token);
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Overrides of the XMLDocumentHandler interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::docCharacters(  const   XMLCh* const    chars
                                , const XMLSize_t       length
                                , const bool            cdataSection)
{
    // Suppress the chars before the root element.
    if (fElemDepth)
    {
        // Call the installed LexicalHandler.
        if (cdataSection && fLexicalHandler)
            fLexicalHandler->startCDATA();

        // Just map to the SAX document handler
        if (fDocHandler)
            fDocHandler->characters(chars, length);

        // Call the installed LexicalHandler.
        if (cdataSection && fLexicalHandler)
            fLexicalHandler->endCDATA();
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docCharacters(chars, length, cdataSection);
}


void SAX2XMLReaderImplLoc::docComment(const XMLCh* const commentText)
{
    // Call the installed LexicalHandler.
    if (fLexicalHandler)
    {
        // SAX2 reports comment text like characters -- as an
        // array with a length.
        fLexicalHandler->comment(commentText, XMLString::stringLen(commentText));
    }

    //
    //  OK, if there are any installed advanced handlers,
    // then let's call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docComment(commentText);
}


void SAX2XMLReaderImplLoc::XMLDecl( const  XMLCh* const    versionStr
                        , const XMLCh* const    encodingStr
                        , const XMLCh* const    standaloneStr
                        , const XMLCh* const    actualEncodingStr
                        )
{
    // SAX has no way to report this event. But, if there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->XMLDecl( versionStr,
                                    encodingStr,
                                    standaloneStr,
                                    actualEncodingStr );
}


void SAX2XMLReaderImplLoc::docPI(  const   XMLCh* const    target
                        , const XMLCh* const    data)
{
    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->processingInstruction(target, data);

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->docPI(target, data);
}


void SAX2XMLReaderImplLoc::endDocument()
{
    if (fDocHandler)
        fDocHandler->endDocument();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endDocument();
}


void SAX2XMLReaderImplLoc::endEntityReference(const XMLEntityDecl& entityDecl)
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
        fLexicalHandler->endEntity(entityDecl.getName());

    //
    //  SAX has no way to report this event. But, if there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endEntityReference(entityDecl);
}


void SAX2XMLReaderImplLoc::ignorableWhitespace(const   XMLCh* const    chars
                                    , const XMLSize_t       length
                                    , const bool            cdataSection)
{
    // Do not report the whitespace before the root element.
    if (!fElemDepth)
        return;

    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->ignorableWhitespace(chars, length);

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->ignorableWhitespace(chars, length, cdataSection);
}


void SAX2XMLReaderImplLoc::resetDocument()
{
    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->resetDocument();

    // Make sure our element depth flag gets set back to zero
    fElemDepth = 0;

    // reset prefix counters and prefix map
    fPrefixCounts->removeAllElements();
    fPrefixes->removeAllElements();
    fPrefixesStorage->flushAll();
}


void SAX2XMLReaderImplLoc::startDocument()
{
    // Just map to the SAX document handler
    if (fDocHandler)
        fDocHandler->setDocumentLocator(fScanner->getLocator());
    if(fDocHandler)
        fDocHandler->startDocument();

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->startDocument();
}


void SAX2XMLReaderImplLoc::
startElement(   const   XMLElementDecl&         elemDecl
                , const unsigned int            elemURLId
                , const XMLCh* const            elemPrefix
                , const RefVectorOf<XMLAttr>&   attrList
                , const XMLSize_t               attrCount
                , const bool                    isEmpty
                , const bool                    isRoot)
{
    // Bump the element depth counter if not empty
    if (!isEmpty)
        fElemDepth++;

    if (fDocHandler)
    {
        const QName* qName=elemDecl.getElementName();
        const XMLCh* baseName=qName->getLocalPart();
        const XMLCh* elemQName = 0;
        if(elemPrefix==0 || *elemPrefix==0)
            elemQName=baseName;
        else if(XMLString::equals(elemPrefix, qName->getPrefix()))
            elemQName=qName->getRawName();
        else
        {
            fTempQName->set(elemPrefix);
            fTempQName->append(chColon);
            fTempQName->append(baseName);
            elemQName=fTempQName->getRawBuffer();
        }

        if (getDoNamespaces())
        {
            XMLSize_t numPrefix = 0;

            if (!fNamespacePrefix)
                fTempAttrVec->removeAllElements();

            for (XMLSize_t i = 0; i < attrCount; i++)
            {
                const XMLCh*   nsPrefix = 0;
                const XMLCh*   nsURI    = 0;

                const XMLAttr* tempAttr = attrList.elementAt(i);
                const XMLCh* prefix = tempAttr->getPrefix();
                if(prefix && *prefix)
                {
                    if(XMLString::equals(prefix, XMLUni::fgXMLNSString))
                    {
                        nsPrefix = tempAttr->getName();
                        nsURI = tempAttr->getValue();
                    }
                }
                else if (XMLString::equals(tempAttr->getName(), XMLUni::fgXMLNSString))
                {
                    nsPrefix = XMLUni::fgZeroLenString;
                    nsURI = tempAttr->getValue();
                }
                if (!fNamespacePrefix)
                {
                    if (nsURI == 0)
                        fTempAttrVec->addElement((XMLAttr*)tempAttr);
                }
                if (nsURI != 0)
                {
                    if(fDocHandler)
                        fDocHandler->startPrefixMapping(nsPrefix, nsURI);
                    unsigned int nPrefixId=fPrefixesStorage->addOrFind(nsPrefix);
                    fPrefixes->push(nPrefixId) ;
                    numPrefix++;
                }
            }
            fPrefixCounts->push(numPrefix) ;
            if (!fNamespacePrefix)
                fAttrList.setVector(fTempAttrVec, fTempAttrVec->size(), fScanner);
            else
                fAttrList.setVector(&attrList, attrCount, fScanner);

            // call startElement() with namespace declarations
            if(fDocHandler)
            {
                fDocHandler->startElement
                (
                    fScanner->getURIText(elemURLId)
                    , baseName
                    , elemQName
                    , fAttrList
                );
            }
        }
        else // no namespace
        {
            fAttrList.setVector(&attrList, attrCount, fScanner);
            if(fDocHandler)
            {
                fDocHandler->startElement(XMLUni::fgZeroLenString,
                                          XMLUni::fgZeroLenString,
                                          qName->getRawName(),
                                          fAttrList);
            }
        }


        // If its empty, send the end tag event now
        if (isEmpty)
        {
            // call endPrefixMapping appropriately.
            if (getDoNamespaces())
            {
                if(fDocHandler)
                {
                    fDocHandler->endElement
                    (
                        fScanner->getURIText(elemURLId)
                        , baseName
                        , elemQName
                    );
                }

                XMLSize_t numPrefix = fPrefixCounts->pop();
                for (XMLSize_t i = 0; i < numPrefix; ++i)
                {
                    unsigned int nPrefixId = fPrefixes->pop() ;
                    if(fDocHandler)
                        fDocHandler->endPrefixMapping( fPrefixesStorage->getValueForId(nPrefixId) );
                }
            }
            else
            {
                if(fDocHandler)
                {
                    fDocHandler->endElement(XMLUni::fgZeroLenString,
                                    XMLUni::fgZeroLenString,
                                    qName->getRawName());
                }
            }
        }
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
    {
        fAdvDHList[index]->startElement
        (
            elemDecl
            , elemURLId
            , elemPrefix
            , attrList
            , attrCount
            , isEmpty
            , isRoot
        );
    }
}

void SAX2XMLReaderImplLoc::endElement( const   XMLElementDecl& elemDecl
                            , const unsigned int    uriId
                            , const bool            isRoot
                            , const XMLCh* const    elemPrefix)
{
    // Just map to the SAX document handler
    if (fDocHandler)
    {
        const QName* qName=elemDecl.getElementName();
        const XMLCh* baseName=qName->getLocalPart();
        const XMLCh* elemQName = 0;
        if(elemPrefix==0 || *elemPrefix==0)
            elemQName=baseName;
        else if(XMLString::equals(elemPrefix, qName->getPrefix()))
            elemQName=qName->getRawName();
        else
        {
            fTempQName->set(elemPrefix);
            fTempQName->append(chColon);
            fTempQName->append(baseName);
            elemQName=fTempQName->getRawBuffer();
        }

        if (getDoNamespaces())
        {
            if(fDocHandler)
            {
                fDocHandler->endElement
                (
                    fScanner->getURIText(uriId)
                    , baseName
                    , elemQName
                );
            }

            // get the prefixes back so that we can call endPrefixMapping()
            XMLSize_t numPrefix = fPrefixCounts->pop();
            for (XMLSize_t i = 0; i < numPrefix; i++)
            {
                unsigned int nPrefixId = fPrefixes->pop() ;
                if(fDocHandler)
                    fDocHandler->endPrefixMapping( fPrefixesStorage->getValueForId(nPrefixId) );
            }
        }
        else
        {
            if(fDocHandler)
            {
              fDocHandler->endElement(XMLUni::fgZeroLenString,
                                      XMLUni::fgZeroLenString,
                                      qName->getRawName());
            }
        }
    }

    //
    //  If there are any installed advanced handlers, then lets call them
    //  with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->endElement(elemDecl, uriId, isRoot, elemPrefix);

    //
    //  Dump the element depth down again. Don't let it underflow in case
    //  of malformed XML.
    //
    if (fElemDepth)
        fElemDepth--;
}

void SAX2XMLReaderImplLoc::startEntityReference(const XMLEntityDecl& entityDecl)
{
   // Call the installed LexicalHandler.
   if (fLexicalHandler)
        fLexicalHandler->startEntity(entityDecl.getName());
    //
    //  SAX has no way to report this. But, If there are any installed
    //  advanced handlers, then lets call them with this info.
    //
    for (XMLSize_t index = 0; index < fAdvDHCount; index++)
        fAdvDHList[index]->startEntityReference(entityDecl);
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Overrides of the DocTypeHandler interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::attDef( const   DTDElementDecl& elemDecl
                        , const DTDAttDef&      attDef
                        , const bool            ignoring)
{
    if (fDeclHandler && !ignoring) {

        XMLAttDef::AttTypes attType = attDef.getType();
        XMLAttDef::DefAttTypes defAttType = attDef.getDefaultType();
        const XMLCh* defAttTypeStr = XMLUni::fgNullString;
        bool isEnumeration = (attType == XMLAttDef::Notation || attType == XMLAttDef::Enumeration);
        XMLBuffer enumBuf(128, fMemoryManager);

        if (defAttType == XMLAttDef::Fixed ||
            defAttType == XMLAttDef::Implied ||
            defAttType == XMLAttDef::Required) {
            defAttTypeStr = attDef.getDefAttTypeString(defAttType, fMemoryManager);
        }

        if (isEnumeration) {

            const XMLCh* enumString = attDef.getEnumeration();
            XMLSize_t enumLen = XMLString::stringLen(enumString);

            if (attType == XMLAttDef::Notation) {

                enumBuf.set(XMLUni::fgNotationString);
                enumBuf.append(chSpace);
            }

            enumBuf.append(chOpenParen);

            for (XMLSize_t i=0; i<enumLen; i++) {
                if (enumString[i] == chSpace)
                    enumBuf.append(chPipe);
                else
                    enumBuf.append(enumString[i]);
            }

            enumBuf.append(chCloseParen);
        }

        fDeclHandler->attributeDecl(elemDecl.getFullName(),
                                    attDef.getFullName(),
                                    (isEnumeration) ? enumBuf.getRawBuffer()
                                                    : attDef.getAttTypeString(attDef.getType(), fMemoryManager),
                                    defAttTypeStr,
                                    attDef.getValue());
    }
}


void SAX2XMLReaderImplLoc::doctypeComment(const XMLCh* const commentText)
{
   if (fLexicalHandler)
   {
        // SAX2 reports comment text like characters -- as an
        // array with a length.
        fLexicalHandler->comment(commentText, XMLString::stringLen(commentText));
   }
}


void SAX2XMLReaderImplLoc::doctypeDecl(const   DTDElementDecl& elemDecl
                            , const XMLCh* const    publicId
                            , const XMLCh* const    systemId
                            , const bool            hasIntSubset
                            , const bool            hasExtSubset)
{
    // Call the installed LexicalHandler.
    if (fLexicalHandler && (hasIntSubset || hasExtSubset))
        fLexicalHandler->startDTD(elemDecl.getFullName(), publicId, systemId);

    fHasExternalSubset = hasExtSubset;

    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::doctypePI(  const   XMLCh* const
                            , const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::doctypeWhitespace(  const   XMLCh* const
                                    , const XMLSize_t)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::elementDecl(const DTDElementDecl& elemDecl,
                                    const bool isIgnored)
{
    if (fDeclHandler && !isIgnored)
        fDeclHandler->elementDecl(elemDecl.getFullName(),
                                  elemDecl.getFormattedContentModel());
}


void SAX2XMLReaderImplLoc::endAttList(const DTDElementDecl&)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::endIntSubset()
{
   // Call the installed LexicalHandler.
   if (!fHasExternalSubset && fLexicalHandler)
        fLexicalHandler->endDTD();

    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::endExtSubset()
{
    // Call the installed LexicalHandler.
    if (fLexicalHandler)
        fLexicalHandler->endEntity(gDTDEntityStr);
    if (fLexicalHandler)
        fLexicalHandler->endDTD();

    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::entityDecl( const   DTDEntityDecl&  entityDecl
                            , const bool            isPEDecl
                            , const bool            isIgnored)
{
    //
    //  If we have a DTD handler, and this entity is not ignored, and
    //  its an unparsed entity, then send this one, else if we have a Decl
    //  handler then send this one.
    //
    if (!isIgnored) {

        if (entityDecl.isUnparsed()) {

            if (fDTDHandler) {
                fDTDHandler->unparsedEntityDecl
                (
                    entityDecl.getName()
                    , entityDecl.getPublicId()
                    , entityDecl.getSystemId()
                    , entityDecl.getNotationName()
                );
            }
        }
        else if (fDeclHandler) {

            const XMLCh* entityName = entityDecl.getName();
            ArrayJanitor<XMLCh> tmpNameJan(0);

            if (isPEDecl) {

                XMLSize_t nameLen = XMLString::stringLen(entityName);
                XMLCh* tmpName = (XMLCh*) fMemoryManager->allocate
                (
                    (nameLen + 2) * sizeof(XMLCh)
                );//new XMLCh[nameLen + 2];

                tmpNameJan.reset(tmpName, fMemoryManager);
                tmpName[0] = chPercent;
                XMLString::copyString(tmpName + 1, entityName);
                entityName = tmpName;
            }

            if (entityDecl.isExternal()) {
                fDeclHandler->externalEntityDecl
                (
                    entityName
                    , entityDecl.getPublicId()
                    , entityDecl.getSystemId()
                );
            }
            else {
                fDeclHandler->internalEntityDecl
                (
                    entityName
                    , entityDecl.getValue()
                );
            }
        }
    }
}


void SAX2XMLReaderImplLoc::resetDocType()
{
    fHasExternalSubset = false;
    // Just map to the DTD handler
    if (fDTDHandler)
        fDTDHandler->resetDocType();
}


void SAX2XMLReaderImplLoc::notationDecl(   const   XMLNotationDecl&    notDecl
                                , const bool                isIgnored)
{
    if (fDTDHandler && !isIgnored)
    {
        fDTDHandler->notationDecl
        (
            notDecl.getName()
            , notDecl.getPublicId()
            , notDecl.getSystemId()
        );
    }
}


void SAX2XMLReaderImplLoc::startAttList(const DTDElementDecl&)
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::startIntSubset()
{
    // Unused by SAX DTDHandler interface at this time
}


void SAX2XMLReaderImplLoc::startExtSubset()
{
    if (fLexicalHandler)
        fLexicalHandler->startEntity(gDTDEntityStr);
}


void SAX2XMLReaderImplLoc::TextDecl(   const  XMLCh* const
                            , const XMLCh* const)
{
    // Unused by SAX DTDHandler interface at this time
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Handlers for the XMLEntityHandler interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::endInputSource(const InputSource&)
{
}

bool SAX2XMLReaderImplLoc::expandSystemId(const XMLCh* const, XMLBuffer&)
{
    return false;
}


void SAX2XMLReaderImplLoc::resetEntities()
{
    // Nothing to do for this one
}

InputSource* SAX2XMLReaderImplLoc::resolveEntity(XMLResourceIdentifier* resourceIdentifier)
{
    //
    //  Just map it to the SAX entity resolver. If there is not one installed,
    //  return a null pointer to cause the default resolution.
    //
    if (fEntityResolver)
        return fEntityResolver->resolveEntity(resourceIdentifier->getPublicId(),
                                                resourceIdentifier->getSystemId());
    if (fXMLEntityResolver)
        return fXMLEntityResolver->resolveEntity(resourceIdentifier);

    return 0;
}

void SAX2XMLReaderImplLoc::startInputSource(const InputSource&)
{
    // Nothing to do for this one
}

// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Overrides of the XMLErrorReporter interface
// ---------------------------------------------------------------------------
void SAX2XMLReaderImplLoc::resetErrors()
{
    if (fErrorHandler)
        fErrorHandler->resetErrors();
}


void SAX2XMLReaderImplLoc::error(  const   unsigned int
                        , const XMLCh* const
                        , const XMLErrorReporter::ErrTypes  errType
                        , const XMLCh* const                errorText
                        , const XMLCh* const                systemId
                        , const XMLCh* const                publicId
                        , const XMLFileLoc                  lineNum
                        , const XMLFileLoc                  colNum)
{
    SAXParseException toThrow = SAXParseException
    (
        errorText
        , publicId
        , systemId
        , lineNum
        , colNum
        , fMemoryManager
    );

    if (!fErrorHandler)
    {
        if (errType == XMLErrorReporter::ErrType_Fatal)
            throw toThrow;
        else
            return;
    }

    if (errType == XMLErrorReporter::ErrType_Warning)
        fErrorHandler->warning(toThrow);
    else if (errType == XMLErrorReporter::ErrType_Fatal)
        fErrorHandler->fatalError(toThrow);
    else
        fErrorHandler->error(toThrow);
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Features and Properties
// ---------------------------------------------------------------------------

void SAX2XMLReaderImplLoc::setFeature(const XMLCh* const name, const bool value)
{

    if (fParseInProgress)
        throw SAXNotSupportedException("Feature modification is not supported during parse.", fMemoryManager);

    if (XMLString::compareIStringASCII(name, XMLUni::fgSAX2CoreNameSpaces) == 0)
    {
        setDoNamespaces(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgSAX2CoreValidation) == 0)
    {
        fValidation = value;
        if (fValidation)
            if (fAutoValidation)
                setValidationScheme(Val_Auto);
            else
                setValidationScheme(Val_Always);
        else
            setValidationScheme(Val_Never);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgSAX2CoreNameSpacePrefixes) == 0)
    {
        fNamespacePrefix = value;
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDynamic) == 0)
    {
        fAutoValidation = value;
        // for auto validation, the sax2 core validation feature must also be enabled.
        if (fValidation)
            if (fAutoValidation)
                setValidationScheme(Val_Auto);
            else
                setValidationScheme(Val_Always);
        else
            setValidationScheme(Val_Never);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchema) == 0)
    {
        setDoSchema(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaFullChecking) == 0)
    {
        fScanner->setValidationSchemaFullChecking(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIdentityConstraintChecking) == 0)
    {
        fScanner->setIdentityConstraintChecking(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadExternalDTD) == 0)
    {
        fScanner->setLoadExternalDTD(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadSchema) == 0)
    {
        fScanner->setLoadSchema(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesContinueAfterFatalError) == 0)
    {
        fScanner->setExitOnFirstFatal(!value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidationErrorAsFatal) == 0)
    {
        fScanner->setValidationConstraintFatal(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCacheGrammarFromParse) == 0)
    {
        fScanner->cacheGrammarFromParse(value);

        if (value)
            fScanner->useCachedGrammarInParse(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesUseCachedGrammarInParse) == 0)
    {
        if (value || !fScanner->isCachingGrammarFromParse())
            fScanner->useCachedGrammarInParse(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCalculateSrcOfs) == 0)
    {
        fScanner->setCalculateSrcOfs(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesStandardUriConformant) == 0)
    {
        fScanner->setStandardUriConformant(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesGenerateSyntheticAnnotations) == 0)
    {
        fScanner->setGenerateSyntheticAnnotations(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidateAnnotations) == 0)
    {
        fScanner->setValidateAnnotations(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreCachedDTD) == 0)
    {
        fScanner->setIgnoredCachedDTD(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreAnnotations) == 0)
    {
        fScanner->setIgnoreAnnotations(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDisableDefaultEntityResolution) == 0)
    {
        fScanner->setDisableDefaultEntityResolution(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSkipDTDValidation) == 0)
    {
        fScanner->setSkipDTDValidation(value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesHandleMultipleImports) == 0)
    {
        fScanner->setHandleMultipleImports(value);
    }
    else
       throw SAXNotRecognizedException("Unknown Feature", fMemoryManager);
}

bool SAX2XMLReaderImplLoc::getFeature(const XMLCh* const name) const
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgSAX2CoreNameSpaces) == 0)
        return getDoNamespaces();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgSAX2CoreValidation) == 0)
        return fValidation;
    else if (XMLString::compareIStringASCII(name, XMLUni::fgSAX2CoreNameSpacePrefixes) == 0)
        return fNamespacePrefix;
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDynamic) == 0)
        return fAutoValidation;
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchema) == 0)
        return getDoSchema();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaFullChecking) == 0)
        return fScanner->getValidationSchemaFullChecking();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIdentityConstraintChecking) == 0)
        return fScanner->getIdentityConstraintChecking();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadExternalDTD) == 0)
        return fScanner->getLoadExternalDTD();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLoadSchema) == 0)
        return fScanner->getLoadSchema();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesContinueAfterFatalError) == 0)
        return !fScanner->getExitOnFirstFatal();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidationErrorAsFatal) == 0)
        return fScanner->getValidationConstraintFatal();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCacheGrammarFromParse) == 0)
        return fScanner->isCachingGrammarFromParse();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesUseCachedGrammarInParse) == 0)
        return fScanner->isUsingCachedGrammarInParse();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesCalculateSrcOfs) == 0)
        return fScanner->getCalculateSrcOfs();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesStandardUriConformant) == 0)
        return fScanner->getStandardUriConformant();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesGenerateSyntheticAnnotations) == 0)
        return fScanner->getGenerateSyntheticAnnotations();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesValidateAnnotations) == 0)
        return fScanner->getValidateAnnotations();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreCachedDTD) == 0)
        return fScanner->getIgnoreCachedDTD();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesIgnoreAnnotations) == 0)
        return fScanner->getIgnoreAnnotations();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesDisableDefaultEntityResolution) == 0)
        return fScanner->getDisableDefaultEntityResolution();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSkipDTDValidation) == 0)
        return fScanner->getSkipDTDValidation();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesHandleMultipleImports) == 0)
        return fScanner->getHandleMultipleImports();
    else
       throw SAXNotRecognizedException("Unknown Feature", fMemoryManager);

    return false;
}

void SAX2XMLReaderImplLoc::setProperty(const XMLCh* const name, void* value)
{
    if (fParseInProgress)
        throw SAXNotSupportedException("Property modification is not supported during parse.", fMemoryManager);

    if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalSchemaLocation) == 0)
    {
        fScanner->setExternalSchemaLocation((XMLCh*)value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation) == 0)
    {
        fScanner->setExternalNoNamespaceSchemaLocation((XMLCh*)value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSecurityManager) == 0)
    {
        fScanner->setSecurityManager((SecurityManager*)value);
    }
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLowWaterMark) == 0)
    {
        fScanner->setLowWaterMark(*(const XMLSize_t*)value);
    }
    else if (XMLString::equals(name, XMLUni::fgXercesScannerName))
    {
        XMLScanner* tempScanner = XMLScannerResolver::resolveScanner
        (
            (const XMLCh*) value
            , fValidator
            , fGrammarResolver
            , fMemoryManager
        );

        if (tempScanner) {

            tempScanner->setParseSettings(fScanner);
            tempScanner->setURIStringPool(fURIStringPool);
            delete fScanner;
            fScanner = tempScanner;
        }
    }
    else
       throw SAXNotRecognizedException("Unknown Property", fMemoryManager);
}


void* SAX2XMLReaderImplLoc::getProperty(const XMLCh* const name) const
{
    if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalSchemaLocation) == 0)
        return (void*)fScanner->getExternalSchemaLocation();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSchemaExternalNoNameSpaceSchemaLocation) == 0)
        return (void*)fScanner->getExternalNoNamespaceSchemaLocation();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesSecurityManager) == 0)
        return (void*)fScanner->getSecurityManager();
    else if (XMLString::compareIStringASCII(name, XMLUni::fgXercesLowWaterMark) == 0)
        return (void*)&fScanner->getLowWaterMark();
    else if (XMLString::equals(name, XMLUni::fgXercesScannerName))
        return (void*)fScanner->getName();
    else
        throw SAXNotRecognizedException("Unknown Property", fMemoryManager);
    return 0;
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Private getters and setters for conveniences
// ---------------------------------------------------------------------------

void SAX2XMLReaderImplLoc::setValidationScheme(const ValSchemes newScheme)
{
    if (newScheme == Val_Never)
        fScanner->setValidationScheme(XMLScanner::Val_Never);
    else if (newScheme == Val_Always)
        fScanner->setValidationScheme(XMLScanner::Val_Always);
    else
        fScanner->setValidationScheme(XMLScanner::Val_Auto);
}

void SAX2XMLReaderImplLoc::setDoNamespaces(const bool newState)
{
    fScanner->setDoNamespaces(newState);
}

bool SAX2XMLReaderImplLoc::getDoNamespaces() const
{
    return fScanner->getDoNamespaces();
}

void SAX2XMLReaderImplLoc::setDoSchema(const bool newState)
{
    fScanner->setDoSchema(newState);
}

bool SAX2XMLReaderImplLoc::getDoSchema() const
{
    return fScanner->getDoSchema();
}


// ---------------------------------------------------------------------------
//  SAX2XMLReaderImplLoc: Grammar preparsing
// ---------------------------------------------------------------------------
Grammar* SAX2XMLReaderImplLoc::loadGrammar(const char* const systemId,
                                        const Grammar::GrammarType grammarType,
                                        const bool toCache)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAX2XMLReaderImplLoc::resetInProgress);

    Grammar* grammar = 0;
    try
    {
        fParseInProgress = true;
        grammar = fScanner->loadGrammar(systemId, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }

    return grammar;
}

Grammar* SAX2XMLReaderImplLoc::loadGrammar(const XMLCh* const systemId,
                                        const Grammar::GrammarType grammarType,
                                        const bool toCache)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAX2XMLReaderImplLoc::resetInProgress);

    Grammar* grammar = 0;
    try
    {
        fParseInProgress = true;
        grammar = fScanner->loadGrammar(systemId, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }

    return grammar;
}

Grammar* SAX2XMLReaderImplLoc::loadGrammar(const InputSource& source,
                                        const Grammar::GrammarType grammarType,
                                        const bool toCache)
{
    // Avoid multiple entrance
    if (fParseInProgress)
        ThrowXMLwithMemMgr(IOException, XMLExcepts::Gen_ParseInProgress, fMemoryManager);

    ResetInProgressType     resetInProgress(this, &SAX2XMLReaderImplLoc::resetInProgress);

    Grammar* grammar = 0;
    try
    {
        fParseInProgress = true;
        grammar = fScanner->loadGrammar(source, grammarType, toCache);
    }
    catch(const OutOfMemoryException&)
    {
        resetInProgress.release();

        throw;
    }

    return grammar;
}

void SAX2XMLReaderImplLoc::resetInProgress()
{
    fParseInProgress = false;
}

void SAX2XMLReaderImplLoc::resetCachedGrammarPool()
{
    fGrammarResolver->resetCachedGrammar();
    fScanner->resetCachedGrammar();
}

void SAX2XMLReaderImplLoc::setInputBufferSize(const XMLSize_t bufferSize)
{
    fScanner->setInputBufferSize(bufferSize);
}

Grammar* SAX2XMLReaderImplLoc::getGrammar(const XMLCh* const nameSpaceKey)
{
    return fGrammarResolver->getGrammar(nameSpaceKey);
}


XERCES_CPP_NAMESPACE_END
