/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
try {
    var readability = {}
regexps = {
    unlikelyCandidates:    /combx|comment|community|disqus|extra|foot|header|menu|remark|rss|shoutbox|sidebar|sponsor|ad-break|agegate|pagination|pager|popup|tweet|twitter/i,
    okMaybeItsACandidate:  /and|article|body|column|main|shadow/i,
    positive:              /article|body|content|entry|hentry|main|page|pagination|post|text|blog|story|windowclassic/i,
    negative:              /contents|combx|comment|com-|contact|foot|footer|footnote|masthead|media|meta|outbrain|promo|related|scroll|shoutbox|sidebar|date|sponsor|shopping|tags|script|tool|widget|scbox|rail|reply|div_dispalyslide|galleryad|disqus_thread|cnn_strylftcntnt|topRightNarrow|fs-stylelist-thumbnails|replText|ttalk_layer|disqus_post_message|disqus_post_title|cnn_strycntntrgt|wpadvert|sharedaddy sd-like-enabled sd-sharing-enabled|fs-slideshow-wrapper|fs-stylelist-launch|fs-stylelist-next|fs-thumbnail-194230|reply_box|textClass errorContent|mainHeadlineBrief|mainSlideDetails|curvedContent|photo|home_|XMOD/i,
    extraneous:            /print|archive|comment|discuss|e[\-]?mail|share|reply|all|login|sign|single/i,
    divToPElements:        /<(a|blockquote|dl|div|img|ol|p|pre|table|ul|script|article)/i,
    replaceBrs:            /(<br[^>]*>[ \n\r\t]*){2,}/gi,
    replaceFonts:          /<(\/?)font[^>]*>/gi,
    trim:                  /^\s+|\s+$/g,
    normalize:             /\s{2,}/g,
    killBreaks:            /(<br\s*\/?>(\s|&nbsp;?)*){1,}/g,
    videos:                /http:\/\/(www\.)?(youtube|vimeo)\.com/i,
    skipFootnoteLink:      /^\s*(\[?[a-z0-9]{1,2}\]?|^|edit|citation needed)\s*$/i,
    nextLink:              /(next|weiter|continue|>([^\|]|$)|([^\|]|$))/i,
    prevLink:              /(prev|earl|old|new|<|)/i,
    homePage:          /(\?mview=desktop|\?ref=smartphone|apple.com|query=|search\?|\?from=mobile|signup|twitter|facebook|linkedin|dromaeo|gsshop|cinderella|gdive)/i
}

function getLinkDensity(e) {
    var links      = e.getElementsByTagName("a");
    var textLength = getInnerText(e).length;
    var linkLength = 0;

    for (var i=0, il=links.length; i<il;i+=1)
        linkLength += getInnerText(links[i]).length;

    return linkLength / textLength;
}

function getClassWeight(e) {
    var weight = 0;

    if (typeof(e.className) === 'string' && e.className !== '') {
        if (e.className.search(regexps.negative) !== -1)
            weight -= 25;

        if (e.className.search(regexps.positive) !== -1)
            weight += 30;
    }

    if (typeof(e.id) === 'string' && e.id !== '') {
        if (e.id.search(regexps.negative) !== -1)
            weight -= 25;

        if (e.id.search(regexps.positive) !== -1)
            weight += 30;
    }

    return weight;
}

function initializeNode(node) {
    node.readability = {"contentScore": 0};

    switch(node.tagName) {
        case 'DIV':
            node.readability.contentScore += 5;
            break;
        case 'ARTICLE':
            node.readability.contentScore += 25;
            break;
        case 'PRE':
        case 'TD':
        case 'BLOCKQUOTE':
            node.readability.contentScore += 3;
            break;
        case 'ADDRESS':
        case 'OL':
        case 'UL':
        case 'DL':
        case 'DD':
        case 'DT':
        case 'LI':
        case 'FORM':
            node.readability.contentScore -= 3;
            break;
        case 'H1':
        case 'H2':
        case 'H3':
        case 'H4':
        case 'H5':
        case 'H6':
        case 'TH':
            node.readability.contentScore -= 5;
            break;
    }

    node.readability.contentScore += getClassWeight(node);
}
//>> SAMSUNG CHANGE - MPSG5740 - This innerText calculation function has been added because on some sites, the content of the script tag is considered as present inside a text node. So when the default innerText function is evaluated in our case, the inline script itself is evaluated as readable content (thus adding the parent, grandparent to the candidates array when it should not be.

function getActualInnerText(node) {
    if (node.parentNode && node.parentNode.tagName === 'SCRIPT')//implies that this is actually the content of the script node which we should not consider
        return "";

    if (node.nodeType == 3)
        return node.data;

        var ActualInnerText = "";

        node = node.firstChild;

        while(node) {
            ActualInnerText = ActualInnerText + getActualInnerText(node);
            node = node.nextSibling;
        }

   return ActualInnerText;
}

//<< SAMSUNG CHANGE - MPSG5740
function getInnerText(e, normalizeSpaces) {
    var textContent    = "";

    if (typeof(e.textContent) === "undefined" && typeof(e.innerText) === "undefined")
        return "";

    normalizeSpaces = (typeof normalizeSpaces === 'undefined') ? true : normalizeSpaces;

    if (navigator.appName === "Microsoft Internet Explorer") {
        textContent = e.innerText.replace( regexps.trim, "" );
    } else {
//>>SAMSUNG CHANGE - MPSG5740
        textContent = getActualInnerText(e);
    //  textContent = e.textContent.replace( regexps.trim, "" );
//<<SAMSUNG CHANGE - MPSG5740
    }

//>>SAMSUNG CHANGE - MPSG5740
    if (normalizeSpaces && typeof(textContent) !== "undefined") {
//<<SAMSUNG CHANGE - MPSG5740
        return textContent.replace( regexps.normalize, " ");
    } else {
        return textContent;
    }
}

function ChineseJapneseKorean(innerCharacter) {
    if (!innerCharacter || innerCharacter.length == 0) return false;

    var innerCharacterCode = innerCharacter.charCodeAt(0);
    if (innerCharacterCode > 11904 && innerCharacterCode < 12031) return true;
    if (innerCharacterCode > 12352 && innerCharacterCode < 12543) return true;
    if (innerCharacterCode > 12736 && innerCharacterCode < 19903) return true;
    if (innerCharacterCode > 19968 && innerCharacterCode < 40959) return true;
    if (innerCharacterCode > 44032 && innerCharacterCode < 55215) return true;
    if (innerCharacterCode > 63744 && innerCharacterCode < 64255) return true;
    if (innerCharacterCode > 65072 && innerCharacterCode < 65103) return true;
    if (innerCharacterCode > 131072 && innerCharacterCode < 173791) return true;
    if (innerCharacterCode > 194560 && innerCharacterCode < 195103) return true;
    return false;
}
//>> SAMSUNG CHANGE - MPSG5840 - This function is added because in somesites if the gap ( blank spaces ) between two words are more than a single space. Then we have to consider only one space irrespective of the number of spaces present between two words.
function getActualSplitLength(splitlength,readerText) {

    for (var t = 0; t < readerText.length - 1; t++) {
        if (readerText[t]==' ') {
            if (readerText[t]== readerText[t+1])
                splitlength = splitlength - 1 ;
        }
    }

    return splitlength;
}

//<< SAMSUNG CHANGE - MPSG5840

function recognizeArticle() {
    var CJK;
    var mainBody = document.body;

    if (mainBody === null || mainBody === "undefined") {
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
        //console.log("Reader :: Recognize :: INFO - mainBody is NULL");
//SAMSUNG CHANGE - MPSG5927<<
        return "false";
    }
//SAMSUNG CHANGE - MPSG6011 >> cloneNode is causing a problem with form state which is triggering a timeout during form submission
//when the action url content is same as the initial url content and using a 1-time key generator. As recognizeArticle.js only parses and uses
//the content to decide whether it is readable or not, it is okay to use the original content. cloneNode will be required in the reader.js case
//since we are actually modifying the content in the reader.js parsing.
//NOTE - In case of future issues where disabling reader doesnt cause the issue, then this is first place to check by reverting to the older
//revision of mainBody.cloneNode(true)
    var page = mainBody;
    //var page = mainBody.cloneNode(true);
//SAMSUNG CHANGE - MPSG6011 <<
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
    //console.log("Reader :: Recognize :: INFO - Script is evaluating. recognizeArticle Start");
//SAMSUNG CHANGE - MPSG5927<<

//SAMSUNG CHANGE - MPSG6055 >> For Jellybean, there has been changes in the security origin of the location object which is causing an invalid
//access error for popups - so we are using equivalent functionality from the document object attributes instead of the earlier location properties
//This allows us to resolve the error as well as maintain functionality
    var hostName = document.domain;
    //var hostName = document.location.hostname;
//SAMSUNG CHANGE - MPSG6055<<

    hostName = hostName + "/";

//SAMSUNG CHANGE - MPSG6055 >>
    var hrefName = document.URL;
    //var hrefName = window.location.href;
//SAMSUNG CHANGE - MPSG6055 <<

    var hrefPage = hrefName.replace(/http:\/\/|https:\/\//i, "");
    if (hostName === hrefPage ||  hrefPage.search(regexps.homePage) !== -1) {
        return "false";
    }

    var allElements = page.getElementsByTagName('*');
    var node = null;
    var nodesToScore = [];

    for (var nodeIndex = 0; (node = allElements[nodeIndex]); nodeIndex+=1) {
        var unlikelyMatchString = node.className + node.id;

        if (unlikelyMatchString !== "undefined") {
            if (unlikelyMatchString.search(regexps.unlikelyCandidates) !== -1 && unlikelyMatchString.search(regexps.okMaybeItsACandidate) === -1 && node.tagName !== "BODY")
                continue;
        }
   //SAMSUNG CHANGE MPSG5564 - We should first check for a nested table and if this is so, we should only take the innermost table, not any of the outer table
        if (node.tagName === "P" || node.tagName === "UL" ||( (node.tagName === "TD") && (node.getElementsByTagName('TABLE').length === 0)) || node.tagName === "PRE"||node.tagName === "p" || node.tagName === "ul" ||( (node.tagName === "td") && (node.getElementsByTagName('table').length === 0)) || node.tagName === "pre")
            nodesToScore[nodesToScore.length] = node;
        if (node.tagName === "DIV") {
            if (node.innerHTML.search(regexps.divToPElements) === -1) {
                try {
                    nodesToScore[nodesToScore.length] = node;
                }
                catch(e) {
                }
            } else {
                for (var i = 0, il = node.childNodes.length; i < il; i+=1) {
                    var childNode = node.childNodes[i];
                    if (childNode.nodeType === 3)
                        nodesToScore[nodesToScore.length] = childNode;
                }
            }
        }
    }

    var candidates = [];

    //TIZEN BROWSER CHANGE - WEB-2578 >> This code is initializes the variables that are not initialized by the cache.
    for (var pt=0; pt < nodesToScore.length; pt+=1)
        nodesToScore[pt].parentNode.readability = undefined;
    //TIZEN BROWSER CHANGE - WEB-2578 <<

    for (var pt=0; pt < nodesToScore.length; pt+=1) {
        var parentNode      = nodesToScore[pt].parentNode;
        var grandParentNode = parentNode ? parentNode.parentNode : null;
        var innerText       = getInnerText(nodesToScore[pt]);

        if (!parentNode || typeof(parentNode.tagName) === 'undefined')
            continue;

        if (innerText.length < 30)
            continue;

        if (typeof parentNode.readability === 'undefined') {
            initializeNode(parentNode);
            candidates.push(parentNode);
        }

        if (grandParentNode && typeof(grandParentNode.readability) === 'undefined' && typeof(grandParentNode.tagName) !== 'undefined') {
            initializeNode(grandParentNode);
            candidates.push(grandParentNode);
        }

        var contentScore = 0;
        contentScore+=1;
        contentScore += innerText.split(',').length;

        var innerTextLength = 0;
        for (var i = 0; i < innerText.length; i++) {
            innerCharacter = innerText[i];
            if (ChineseJapneseKorean(innerCharacter) == true) {
                innerTextLength++;
                CJK = true;
            }
        }
        if (CJK) {
            contentScore += Math.min(Math.floor(innerText.length / 100), 3);
            contentScore = contentScore * 3;
        } else {
            if (innerText.length < 25)
                continue;
            contentScore += Math.min(Math.floor(innerText.length / 100), 3);
        }
        parentNode.readability.contentScore += contentScore;

        if (grandParentNode)
            grandParentNode.readability.contentScore += contentScore/2;
    }

    var topCandidate = null;
    for (var c=0, cl=candidates.length; c < cl; c+=1) {
        candidates[c].readability.contentScore = candidates[c].readability.contentScore * (1-getLinkDensity(candidates[c]));

        if (!topCandidate || candidates[c].readability.contentScore > topCandidate.readability.contentScore)
            topCandidate = candidates[c];
    }
//>> SAMSUNG CHANGE - MPSG5711 - PART3 - after we find top candidates, we check how many similar top-candidates were within a 15% range of this top-candidate - this is needed because on homepages, there are several possible topCandidates which differ by a minute amount in score. The check can be within a 10% range, but to be on the safe-side we are using 15%.
//>> SAMSUNG CHANGE - MPSG5711 - PART3 - Usually, for proper article pages, a clear, definitive topCandidate will be present.
      var neighbourCandidates = 0;
    for (var c=0, cl=candidates.length ; c < cl && topCandidate; c+=1) {
//SAMSUNG CHANGE - MPSG5836 >> We should not count the topCandidate when checking for neighbouring candidates
        if ((candidates[c].readability.contentScore >= topCandidate.readability.contentScore*0.85) && (candidates[c] !== topCandidate)) {
//SAMSUNG CHANGE - MPSG5836 <<
            neighbourCandidates++;
        }
    }
//<<SAMSUNG CHANGE - MPSG5711 - PART3

    var numberOfTrs = 0;
    if (topCandidate != null || topCandidate != undefined) {
        if (topCandidate.tagName === "TR" || topCandidate.tagName === "TBODY") {
            topcandidateTR = topCandidate.getElementsByTagName("tr");
            numberOfTrs = topcandidateTR.length;
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
        //console.log("Reader :: Recognize :: INFO - Entire article is present in tr .");
        //console.log("Reader :: Recognize :: DEBUG - Number of tr in the top candidate = " +topcandidateTR.length);
//SAMSUNG CHANGE - MPSG5927<<
        }
    }

//>>SAMSUNG CHANGE - MPSG5711 - PART3 - For now, the check for neighbourCandidates has threshold of 2, it can be modified later as and when required.
    if (neighbourCandidates >=2) {
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
         //console.log("Possible homepage - so disabling reader icon");
//SAMSUNG CHANGE - MPSG5927<<
    }
//SAMSUNG CHANGE - MPSG5560>> - Check for the linkdensity in topcandidate.
    else if((getLinkDensity(topCandidate)) > 0.5 && (getClassWeight(topCandidate)) < 25) {
        //console.log("Reader :: Recognize ::Disabling reader icon as its linkdensity is more. ");
    }
//SAMSUNG CHANGE - MPSG5560<<
//<<SAMSUNG CHANGE - MPSG5711 - PART3
    else if (topCandidate === null || topCandidate.tagName === "BODY" || topCandidate.tagName === "FORM") {
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
        //console.log("Reader :: Recognize :: INFO - No top candidate found . Hence disabling reader icon");
//SAMSUNG CHANGE - MPSG5927<<
    } else {
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasonss
        //console.log("Reader :: Recognize :: INFO - Score  = "+topCandidate.readability.contentScore);
//SAMSUNG CHANGE - MPSG5927<<
        var splitLength = topCandidate.innerText.split(' ').length;
        var readerScore = topCandidate.readability.contentScore;
        var readerTrs = numberOfTrs;
        var readerTextlength = topCandidate.innerText.length;
        var readerPlength = topCandidate.getElementsByTagName("p").length;

        //>> SAMSUNG CHANGE - MPSG5840
        var readerText = topCandidate.innerText;
        splitLength = getActualSplitLength(splitLength,readerText);
        //<< SAMSUNG CHANGE - MPSG5840

        //>> SAMSUNG CHANGE - MPSG5760 - Even after calculating the proper splitlength , Reader icon was displayed for some homepage.
        //SAMSUNG CHANGE - MPSG6533 >>
        if ((readerScore >= 40 && readerTrs < 3 ) || (readerScore >= 20 && readerScore < 30 && readerTextlength >900 && readerPlength >=2 && readerTrs < 3 && !CJK) || (readerScore >= 20 && readerScore < 30 && readerTextlength >1900 && readerPlength >=0 && readerTrs < 3 && !CJK) || (readerScore > 15 && readerScore <=40  && splitLength >=100 && readerTrs < 3 ) || (readerScore >= 100 && readerTextlength >2000  && splitLength >=250 && readerTrs > 200))
        //SAMSUNG CHANGE - MPSG6553 <<
        //<< SAMSUNG CHANGE - MPSG5760.
        {
            for (var pt=0; pt < nodesToScore.length; pt+=1) {
                var parentNode      = nodesToScore[pt].parentNode;
                var grandParentNode = parentNode ? parentNode.parentNode : null;

                if (grandParentNode !== null) {
                    delete parentNode.readability;
                    delete grandParentNode.readability;
                }
            }
            if (readerScore >= 40 && readerTextlength < 100) {
                return "false";
            } else {
                return "true";
            }
        } else {
            return "false";
        }
    }
    return "false";
}
recognizeArticle();
}
catch(e) {
//SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
//console.log("Reader Catch block- Recognizearticle.js");
//SAMSUNG CHANGE - MPSG5927
}
