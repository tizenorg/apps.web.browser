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
    var test;
    var sample = null;

    totalPageNum = 1;

    var readability = {}
    var page = null;
    parsedPages = {};
    pageETags = {};

    function findBaseUrl() {
        var noUrlParams = window.location.pathname.split("?")[0],
            urlSlashes = noUrlParams.split("/").reverse(),
            cleanedSegments = [],
            possibleType = "";
        for (var i = 0, slashLen = urlSlashes.length; i < slashLen; i += 1) {
            var segment = urlSlashes[i];
            if (segment.indexOf(".") !== -1) {
                possibleType = segment.split(".")[1];
                if (!possibleType.match(/[^a-zA-Z]/)) {
                    segment = segment.split(".")[0];
                }
            }
            if (segment.indexOf(',00') !== -1) {
                segment = segment.replace(',00', '');
            }
            if (segment.match(/((_|-)?p[a-z]*|(_|-))[0-9]{1,2}$/i) && ((i === 1) || (i === 0))) {
                segment = segment.replace(/((_|-)?p[a-z]*|(_|-))[0-9]{1,2}$/i, "");
            }
            var del = false;
            if (i < 2 && segment.match(/^\d{1,2}$/)) {
                del = true;
            }
            if (i === 0 && segment.toLowerCase() === "index") {
                del = true;
            }
            if (i < 2 && segment.length < 3 && !urlSlashes[0].match(/[a-z]/i)) {
                del = true;
            }
            if (!del) {
                cleanedSegments.push(segment);
            }
        }
        return window.location.protocol + "//" + window.location.host + cleanedSegments.reverse().join("/");
    }

    function findNextPageLink(elem) {
        var possiblePages = {},
            allLinks = elem.getElementsByTagName('a'),
            articleBaseUrl = findBaseUrl();
        for (var i = 0, il = allLinks.length; i < il; i += 1) {
            var link = allLinks[i],
                linkHref = allLinks[i].href.replace(/#.*$/, '').replace(/\/$/, '');
            if (linkHref === "" || linkHref === articleBaseUrl || linkHref === window.location.href || linkHref in parsedPages) {
                continue;
            }
            if (window.location.host !== linkHref.split(/\/+/g)[1]) {
                continue;
            }
            var linkText = getInnerText(link);
            if (linkText.match(regexps.extraneous)) {
                continue;
            }
            var linkHrefLeftover = linkHref.replace(articleBaseUrl, '');
            if (!linkHrefLeftover.match(/\d/)) {
                continue;
            }
            if (!(linkHref in possiblePages)) {
                possiblePages[linkHref] = {
                    "score": 0,
                    "linkText": linkText,
                    "href": linkHref
                };
            } else {
                possiblePages[linkHref].linkText += ' | ' + linkText;
            }
            var linkObj = possiblePages[linkHref];
            if (linkHref.indexOf(articleBaseUrl) !== 0) {
                linkObj.score -= 25;
            }
            var linkData = linkText + ' ' + link.className + ' ' + link.id + link.innerHTML;
            if (linkData.match(regexps.nextLink)) {
                linkObj.score += 50;
            }
            if (linkData.match(/pag(e|ing|inat)/i)) {
                linkObj.score += 25;
            }
            if (linkData.match(/(first|last)/i)) {
                if (!linkObj.linkText.match(regexps.nextLink)) {
                    linkObj.score -= 65;
                }
            }
            if (linkData.match(regexps.negative) || linkData.match(regexps.extraneous)) {
                linkObj.score -= 50;
            }
            if (linkData.match(regexps.prevLink)) {
                linkObj.score -= 200;
            }
            var parentNode = link.parentNode,
                positiveNodeMatch = false,
                negativeNodeMatch = false;
            while (parentNode) {
                var parentNodeClassAndId = parentNode.className + ' ' + parentNode.id;
                if (!positiveNodeMatch && parentNodeClassAndId && parentNodeClassAndId.match(/pag(e|ing|inat)/i)) {
                    positiveNodeMatch = true;
                    linkObj.score += 15;
                }
                if (!negativeNodeMatch && parentNodeClassAndId && parentNodeClassAndId.match(regexps.negative)) {
                    if (!parentNodeClassAndId.match(regexps.positive)) {
                        linkObj.score -= 25;
                        negativeNodeMatch = true;
                    }
                }
                parentNode = parentNode.parentNode;
            }
            if (linkHref.match(/p(a|g|ag)?(e|ing|ination)?(=|\/)[0-9]{1,2}/i) || linkHref.match(/(page|paging)/i)) {
                linkObj.score += 25;
            }
            if (linkHref.match(regexps.extraneous)) {
                linkObj.score -= 15;
            }
            var linkTextAsNumber = parseInt(linkText, 10);
            if (linkTextAsNumber) {
                if (linkTextAsNumber === 1) {
                    linkObj.score -= 10;
                } else {
                    linkObj.score += Math.max(0, 10 - linkTextAsNumber);
                }
            }
        }
        var topPage = null;
        for (var page in possiblePages) {
            if (possiblePages.hasOwnProperty(page)) {
                if (possiblePages[page].score > 100 && (!topPage || topPage.score < possiblePages[page].score)) {
                    topPage = possiblePages[page];
                }
            }
        }
        if (topPage) {
            var nextHref = topPage.href.replace(/\/$/, '');
            parsedPages[nextHref] = true;
            return nextHref;
            //SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
            //           console.log("Reader :: reader.js :: INFO - Second Page link = "+nextHref);
            //SAMSUNG CHANGE - MPSG5927<<
        } else {
            //SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
            //            console.log("Reader :: reader.js :: INFO - Second Page link = null");
            //SAMSUNG CHANGE - MPSG5927<<
            return null;
        }
    }

    function xhr() {
        if (typeof XMLHttpRequest !== 'undefined' && (window.location.protocol !== 'file:' || !window.ActiveXObject)) {
            return new XMLHttpRequest();
        } else {
            try {
                return new ActiveXObject('Msxml2.XMLHTTP.6.0');
            } catch(sixerr) {}
            try {
                return new ActiveXObject('Msxml2.XMLHTTP.3.0');
            } catch(threrr) {}
            try {
                return new ActiveXObject('Msxml2.XMLHTTP');
            } catch(err) {}
        }
        return false;
    }

    function successfulRequest(request) {
        return (request.status >= 200 && request.status < 300) || request.status === 304 || (request.status === 0 && request.responseText);
    }

    function ajax(url, options) {
        var request = xhr();
        function respondToReadyState(readyState) {
            if (request.readyState === 4) {
                if (successfulRequest(request)) {
                    if (options.success) {
                        options.success(request);
                    }
                } else {
                    if (options.error) {
                        options.error(request);
                    }
                }
            }
        }
        if (typeof options === 'undefined') {
            options = {};
        }
        request.onreadystatechange = respondToReadyState;
        request.open('get', url, true);
        request.setRequestHeader('Accept', 'text/html');
        try {
            request.send(options.postBody);
        } catch (e) {
            if (options.error) {
                options.error();
            }
        }
        return request;
    }
    curPageNum = 1;
    maxPages = 30;

    function appendNextPage(nextPageLink) {
        curPageNum += 1;
        var articlePage = document.createElement("DIV");
        articlePage.id = 'readability-page-' + curPageNum;
        articlePage.className = 'SISO_page';
        articlePage.innerHTML = '<p class="SISO_page-separator" title="Page ' + curPageNum + '">Page ' + curPageNum + '</p>';
        if (curPageNum > maxPages) {
            var nextPageMarkup = "<div style='text-align: center'><a href='" + nextPageLink + "'>View Next Page</a></div>";
            articlePage.innerHTML = articlePage.innerHTML + nextPageMarkup;
            return;
        }
        (function (pageUrl, thisPage) {
            ajax(pageUrl, {
                success: function (r) {
                    var eTag = r.getResponseHeader('ETag');
                    if (eTag) {
                        if (eTag in pageETags) {
                            dbg("Exact duplicate page found via ETag. Aborting.");
                            articlePage.style.display = 'none';
                            return;
                        } else {
                            pageETags[eTag] = 1;
                        }
                    }
                    var page = document.createElement("DIV");
                    var responseHtml = r.responseText.replace(/\n/g, '\uffff').replace(/<script.*?>.*?<\/script>/gi, '');
                    responseHtml = responseHtml.replace(/\n/g, '\uffff').replace(/<script.*?>.*?<\/script>/gi, '');
                    responseHtml = responseHtml.replace(/\uffff/g, '\n').replace(/<(\/?)noscript/gi, '<$1div');
                    responseHtml = responseHtml.replace(regexps.replaceBrs, '</p><p>');
                    responseHtml = responseHtml.replace(regexps.replaceFonts, '<$1span>');
                    page.innerHTML = responseHtml;
                    flags = 0x1 | 0x2 | 0x4;
                    var nextPageLink = findNextPageLink(page),
                        content = grabArticle(page);
                    if (!content) {
                        return;
                    }
                    thisPage.innerHTML += content;
                    document.getElementById("reader_content_div").appendChild(thisPage);
                    if (nextPageLink) {
                        appendNextPage(nextPageLink);
                    }
                }
            });
        }(nextPageLink, articlePage));
    }

    regexps = {
        unlikelyCandidates: /combx|comment|community|disqus|extra|foot|header|menu|remark|rss|shoutbox|sidebar|sponsor|ad-break|agegate|pagination|pager|popup|tweet|twitter/i,
        okMaybeItsACandidate: /and|article|body|column|main|shadow/i,
        positive: /article|body|content|entry|hentry|main|page|pagination|post|text|blog|story|date/i,
        negative: /combx|comment|com-|contact|foot|footer|footnote|masthead|media|meta|outbrain|promo|related|scroll|shoutbox|sidebar|sponsor|shopping|tags|script|tool|widget|scbox|reply|div_dispalyslide|galleryad|disqus_thread|cnn_strylftcntnt|topRightNarrow|fs-stylelist-thumbnails|replText|ttalk_layer|disqus_post_message|disqus_post_title|cnn_strycntntrgt|wpadvert|sharedaddy sd-like-enabled sd-sharing-enabled|fs-slideshow-wrapper|fs-stylelist-launch|reply_box|contentHeader|jive-paginator lw-label/i,
        extraneous: /print|archive|comment|discuss|e[\-]?mail|share|reply|all|login|sign|single/i,
        divToPElements: /<(a|blockquote|dl|div|img|ol|p|pre|table|ul|article)/i,
        replaceBrs: /(<br[^>]*>[ \n\r\t]*){2,}/gi,
        replaceFonts: /<(\/?)font[^>]*>/gi,
        trim: /^\s+|\s+$/g,
        normalize: /\s{2,}/g,
        killBreaks: /(<br\s*\/?>(\s|&nbsp;?)*){1,}/g,
        videos: /http:\/\/(www\.)?(youtube|vimeo)\.com/i,
        skipFootnoteLink: /^\s*(\[?[a-z0-9]{1,2}\]?|^|edit|citation needed)\s*$/i,
        nextLink: /(next|right|weiter|continue|>([^\|]|$)|\u00BB([^\|]|$))/i,
        prevLink: /(prev|earl|old|<|\u226a)/i,
        retainDiv: /whois_record/i
    }

    function cleanHeaders(e) {
        for (var headerIndex = 1; headerIndex < 3; headerIndex += 1) {
            var headers = e.getElementsByTagName('h' + headerIndex);
            for (var i = headers.length - 1; i >= 0; i -= 1) {
                if (getClassWeight(headers[i]) < 0 || getLinkDensity(headers[i]) > 0.33) {
                    headers[i].parentNode.removeChild(headers[i]);
                }
            }
        }
    }

    function clean(e, tag) {
        var targetList = e.getElementsByTagName(tag);
        var isEmbed = (tag === 'object' || tag === 'embed');
        for (var y = targetList.length - 1; y >= 0; y -= 1) {
            if (isEmbed) {
                var attributeValues = "";
                for (var i = 0, il = targetList[y].attributes.length; i < il; i += 1) {
                    attributeValues += targetList[y].attributes[i].value + '|';
                }
                if (attributeValues.search(regexps.videos) !== -1) {
                    continue;
                }
                if (targetList[y].innerHTML.search(regexps.videos) !== -1) {
                    continue;
                }
            }
            targetList[y].parentNode.removeChild(targetList[y]);
        }
    }

    function cleanHTML5(e, tag) {
        var targetList = e.getElementsByTagName(tag);
        for (var y = targetList.length - 1; y >= 0; y -= 1) {
            targetList[y].parentNode.removeChild(targetList[y]);
        }
    }

    function cleanSelect(e, tag) {
        var targetList = e.getElementsByTagName(tag);
        for (var y = targetList.length - 1; y >= 0; y -= 1) {
            targetList[y].style.display = 'none';
        }
    }

    function getCharCount(e, s) {
        s = s || ",";
        return getInnerText(e).split(s).length - 1;
    }

    function getClassWeight(e) {
        var weight = 0;
        if (typeof(e.className) === 'string' && e.className !== '') {
            if (e.className.search(regexps.negative) !== -1) {
                weight -= 25;
            }
            if (e.className.search(regexps.positive) !== -1) {
                weight += 25;
            }
        }
        if (typeof(e.id) === 'string' && e.id !== '') {
            if (e.id.search(regexps.negative) !== -1) {
                weight -= 25;
            }
            if (e.id.search(regexps.positive) !== -1) {
                weight += 25;
            }
        }
        return weight;
    }

    function ChineseJapneseKorean(innerCharacter) {
        if (!innerCharacter || innerCharacter.length == 0) return false;
        var innerCharacterCode = innerCharacter.charCodeAt(0);
        if (innerCharacterCode > 11904 && innerCharacterCode < 12031) return true; //CJK Radicals Supplement
        if (innerCharacterCode > 12352 && innerCharacterCode < 12543) return true; //Hiragana
        if (innerCharacterCode > 12736 && innerCharacterCode < 19903) return true;
        if (innerCharacterCode > 19968 && innerCharacterCode < 40959) return true; //CJK Unified Ideographs
        if (innerCharacterCode > 44032 && innerCharacterCode < 55215) return true; //Hangul Syllables
        if (innerCharacterCode > 63744 && innerCharacterCode < 64255) return true; //CJK Compatibility Ideographs
        if (innerCharacterCode > 65072 && innerCharacterCode < 65103) return true; //CJK Compatibility Forms
        if (innerCharacterCode > 131072 && innerCharacterCode < 173791) return true;
        if (innerCharacterCode > 194560 && innerCharacterCode < 195103) return true;
        return false;
    }

    function cleanConditionally(e, tag) {
        var tagsList = e.getElementsByTagName(tag);
        var curTagsLength = tagsList.length;
        for (var i = curTagsLength - 1; i >= 0; i -= 1) {
            var toRemove = false;
            var weight = getClassWeight(tagsList[i]);
            var contentScore = (typeof tagsList[i].readability !== 'undefined') ? tagsList[i].readability.contentScore : 0;
            if (weight + contentScore < 0) {
                if (tag === "div" || tag === "article") {
                    var readerDivClass5 = document.getElementsByClassName("view_cnt");
                    var readerDivClass6 = document.getElementById("description");
                    var readerDivComment = tagsList[i].className + tagsList[i].id;
                    if (readerDivComment.search(regexps.unlikelyCandidates) !== -1) {
                        tagsList[i].parentNode.removeChild(tagsList[i]);
                    }
                    if (readerDivClass5.length > 0) {
                        toRemove = false;
                    }
                    if (readerDivClass6 !== null && readerDivClass6.innerHTML.length > 0) {
                        toRemove = false;
                    }
                } else {
                    tagsList[i].parentNode.removeChild(tagsList[i]);
                }
            }
            //SAMSUNG CHANGE - MPSG5897 PART 1 >> - some image links have a span tag with text specified as a child node of the link.
            //when this span text is specified with a non-null background-image property, it must not come up on the reader content
            //as the main link here is the image and not the text inside the span tag. so we remove the span tag node in this scenario
            else if (contentScore === 0 && tag === "span") {
                if (e.getElementsByTagName("img") && tagsList[i].style.backgroundImage !== 'none') {
                    tagsList[i].parentNode.removeChild(tagsList[i]);
                }
            }
            //SAMSUNG CHANGE - MPSG5897 PART 1<<
            else if (getCharCount(tagsList[i], ',') < 10) {
                var p = tagsList[i].getElementsByTagName("p").length;
                var img = tagsList[i].getElementsByTagName("img").length;
                var li = tagsList[i].getElementsByTagName("li").length - 100;
                var input = tagsList[i].getElementsByTagName("input").length;
                var embedCount = 0;
                var embeds = tagsList[i].getElementsByTagName("embed");
                for (var ei = 0, il = embeds.length; ei < il; ei += 1) {
                    if (embeds[ei].src.search(regexps.videos) === -1) {
                        embedCount += 1;
                    }
                }
                var linkDensity = getLinkDensity(tagsList[i]);
                var contentLength = getInnerText(tagsList[i]).length;
                var finalWord = null;
                var checkCJK = null;
                var checkCJKText = null;
                var checkWordOfTag = function () {
                        checkCJKText = getInnerText(tagsList[i]);
                        for (var h = 0; h < contentLength; h += 1) {
                            if (ChineseJapneseKorean(checkCJKText[h]) === true) {
                                finalWord = checkCJKText[h];
                                checkCJK = true;
                                break;
                            }
                        }
                        return finalWord;
                    }
                if (img > p) {
                    toRemove = true;
                } else if (li > p && tag !== "ul" && tag !== "ol") {
                    toRemove = true;
                } else if (input > Math.floor(p / 3)) {
                    toRemove = true;
                } else if (contentLength < 25 && (img > 2 || img === 0)) {
                    toRemove = true;
                } else if (weight < 25 && linkDensity > 0.2) {
                    toRemove = true;
                } else if (weight >= 25 && linkDensity > 0.5) {
                    toRemove = true;
                } else if ((embedCount === 1 && contentLength < 75) || embedCount > 1) {
                    toRemove = true;
                }
                if ((checkWordOfTag() !== 'undefined') && checkCJK && toRemove) {
                    if (weight < 25 && linkDensity > 0.3) {
                        toRemove = true;
                    } else {
                        toRemove = false;
                    }
                }
                var images = tagsList[i].getElementsByTagName("img");
                for (var im = 0, il = images.length; im < il; im += 1) {
                    if ((images[im].height >= 150) && (images[im].width >= 200)) {
                        toRemove = false;
                        break;
                    }
                }
                if (tag === "div" || tag === "article") {
                    var readerDivClass = document.getElementsByClassName("whois_record");
                    if (readerDivClass.length > 0) {
                        toRemove = false;
                    }
                    var readerDivClass1 = document.getElementsByClassName("sixteen columns hreview hentry hnews full");
                    if (readerDivClass1.length > 0) {
                        toRemove = false;
                    }
                    var readerDivClass2 = document.getElementsByClassName("list");
                    if (readerDivClass2.length > 0) {
                        toRemove = false;
                    }
                    //Samsung Change - Commented for MPSG100005649 ++
                    //Commenting the patch for MPSG100004561 as it is solved by tree cloning patch.
                    //var readerDivClass3 = document.getElementsByClassName("articleContent");
                    //if(readerDivClass3.length > 0) {
                    //toRemove = false;
                    //}
                    //Samsung Change - Commented for MPSG100005649 --
                    //SAMSUNG CHANGE - MPSG5897 BOTH PARTS>>
                    //This earlier change is being commented because it doesn't seem relevant. How does searching the document for className
                    //'content' seem to be related to the decision to remove this node or not?
                    /*var readerDivClass4 = document.getElementsByClassName("content");
            if(readerDivClass4.length > 0) {
                toRemove = false;   
            }*/
                    //SAMSUNG CHANGE - MPSG5897 BOTH PARTS<<
                    var readerDivClass5 = document.getElementsByClassName("view_cnt");
                    if (readerDivClass5.length > 0) {
                        toRemove = false;
                    }
                    var readerDivClass6 = document.getElementById("description");
                    if (readerDivClass6 !== null && readerDivClass6.innerHTML.length > 0) {
                        toRemove = false;
                    }
                }
                if (toRemove) {
                    tagsList[i].parentNode.removeChild(tagsList[i]);
                }
            }
        }
    }

    function killBreaks(e) {
        try {
            e.innerHTML = e.innerHTML.replace(regexps.killBreaks, '<br />');
        } catch (eBreaks) {}
    }

    function cleanStyles(e) {
        e = e || document;
        var cur = e.firstChild;
        if (!e) {
            return;
        }
        if (typeof e.removeAttribute === 'function' && e.className !== 'readability-styled') {
            e.removeAttribute('style');
        }
        while (cur !== null) {
            if (cur.nodeType === 1) {
                if (cur.className !== "readability-styled") {
                    cur.removeAttribute("style");
                }
                cleanStyles(cur);
            }
            cur = cur.nextSibling;
        }
    }

    function cleanLinkHrefs(e) {
        var links = e.getElementsByTagName("a");
        for (var i = 0, il = links.length; i < il; i += 1) {
            //SAMSUNG CHANGE - MPSG5897 PART 1 >>
            if (links[i].getElementsByTagName("img").length > 0) {
                cleanConditionally(links[i], "span");
            }
            //SAMSUNG CHANGE - MPSG5897 PART1 <<
        }
    }

    function cleanImageLinks(e) {
        var links = e.getElementsByTagName("img");
        for (var i = 0, il = links.length; i < il; i += 1) {
            if (links[i].parentNode.tagName === "A") {
                links[i].parentNode.removeAttribute("href");
            }
        }
    }

    function prepArticle(articleContent) {
        cleanStyles(articleContent);
        cleanConditionally(articleContent, "form");
        clean(articleContent, "object");
        cleanHTML5(articleContent, "video");
        cleanHTML5(articleContent, "audio");
        cleanSelect(articleContent, "select");
        if (articleContent.getElementsByTagName('h2').length === 1) {
            clean(articleContent, "h2");
        }
        clean(articleContent, "iframe");
        clean(articleContent, "script");
        clean(articleContent, "style");
        clean(articleContent, "textarea");
        clean(articleContent, "input");
        clean(articleContent, "\n");
        clean(articleContent, "noscript");
        cleanLinkHrefs(articleContent); //SAMSUNG CHANGE - MPSG5897 PART1 
        cleanImageLinks(articleContent);
        cleanHeaders(articleContent);
        cleanConditionally(articleContent, "table");
        cleanConditionally(articleContent, "ul");
        cleanConditionally(articleContent, "div");
        var articleParagraphs = articleContent.getElementsByTagName('p');
        for (var i = articleParagraphs.length - 1; i >= 0; i -= 1) {
            var imgCount = articleParagraphs[i].getElementsByTagName('img').length;
            var embedCount = articleParagraphs[i].getElementsByTagName('embed').length;
            var objectCount = articleParagraphs[i].getElementsByTagName('object').length;
            if (imgCount === 0 && embedCount === 0 && objectCount === 0 && getInnerText(articleParagraphs[i], false) === '') {
                articleParagraphs[i].parentNode.removeChild(articleParagraphs[i]);
            }
        }
        try {
            articleContent.innerHTML = articleContent.innerHTML.replace(/<br[^>]*>\s*<p/gi, '<p');
        } catch (e) {}
    }

    function getLinkDensity(e) {
        var links = e.getElementsByTagName("a");
        var textLength = getInnerText(e).length;
        var linkLength = 0;
        for (var i = 0, il = links.length; i < il; i += 1) {
            linkLength += getInnerText(links[i]).length;
        }
        return linkLength / textLength;
    }

    function initializeNode(node) {
        node.readability = {
            "contentScore": 0
        };
        switch (node.tagName) {
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

    function getInnerText(e, normalizeSpaces) {
        var textContent = "";
        if (typeof(e.textContent) === "undefined" && typeof(e.innerText) === "undefined") {
            return "";
        }
        normalizeSpaces = (typeof normalizeSpaces === 'undefined') ? true : normalizeSpaces;
        //SAMSUNG CHANGE - MPSG6326 >> this cloneNode call would not be needed in reader.js because the initial cloneNode operation done at the start of 
        //grabArticle function takes care of preparing the copy of the DOM tree for future operations
        //e = e.cloneNode(true);
        //SAMSUNG CHANGE - MPSG6326 <<
        if (e.nodeType != 3) {
            clean(e, "script");
        }
        if (navigator.appName === "Microsoft Internet Explorer") {
            textContent = e.innerText.replace(regexps.trim, "");
        } else {
            textContent = e.textContent.replace(regexps.trim, "");
        }
        if (normalizeSpaces) {
            return textContent.replace(regexps.normalize, " ");
        } else {
            return textContent;
        }
    }

    tagRegexps = {
        replaceAudio :            /<audio([\s\S]*?)<(\/?)audio>/gi,
        replaceVideo :            /<video([\s\S]*?)<(\/?)video>/gi,
        replaceNline :            /\n/gi
    }

    function grabArticle(argumentPage) {
        if (!document.body)
        return null;

        var bodyHtml = argumentPage ? argumentPage.innerHTML : document.body.innerHTML;

        if (bodyHtml.search("audio") != -1)
            bodyHtml = bodyHtml.replace(tagRegexps.replaceAudio,"");

        if (bodyHtml.search("video") != -1)
            bodyHtml = bodyHtml.replace(tagRegexps.replaceVideo,"");

        bodyHtml = bodyHtml.replace(tagRegexps.replaceNline,"");

        page = document.createElement("body");
        page.innerHTML = bodyHtml;

        var allElements = page.getElementsByTagName('*');
        var node = null;
        var nodesToScore = [];
        for (var nodeIndex = 0; (node = allElements[nodeIndex]); nodeIndex += 1) {
            var unlikelyMatchString = node.className + node.id;
            if (unlikelyMatchString !== "undefined") {
                if (unlikelyMatchString.search(regexps.unlikelyCandidates) !== -1 && node.tagName !== "BODY") {
                    continue;
                }
            }
            //SAMSUNG CHANGE MPSG5564 - We should first check for a nested table and if this is so, we should only take the innermost table, not any of the outer table
            if (node.tagName === "P" || ((node.tagName === "TD") && (node.getElementsByTagName('TABLE').length === 0)) || node.tagName === "PRE" || node.tagName === "p" || ((node.tagName === "td") && (node.getElementsByTagName('table').length === 0)) || node.tagName === "pre") {
                nodesToScore[nodesToScore.length] = node;
            }
            if (node.tagName === "DIV") {
                if (node.innerHTML.search(regexps.divToPElements) === -1) {
                    try {
                        nodesToScore[nodesToScore.length] = node;
                    } catch(e) {}
                } else {
                    for (var i = 0, il = node.childNodes.length; i < il; i += 1) {
                        var childNode = node.childNodes[i];
                        if (childNode.nodeType === 3) {
                            nodesToScore[nodesToScore.length] = childNode;
                        }
                    }
                }
            }
        }
        var candidates = [];
        for (var pt = 0; pt < nodesToScore.length; pt += 1) {
            var parentNode = nodesToScore[pt].parentNode;
            var grandParentNode = parentNode ? parentNode.parentNode : null;
            var innerText = getInnerText(nodesToScore[pt]);
            if (!parentNode || typeof(parentNode.tagName) === 'undefined') {
                continue;
            }
            if (innerText.length < 25) {
                continue;
            }
            if (typeof parentNode.readability === 'undefined') {
                initializeNode(parentNode);
                candidates.push(parentNode);
            }
            if (grandParentNode && typeof(grandParentNode.readability) === 'undefined' && typeof(grandParentNode.tagName) !== 'undefined') {
                initializeNode(grandParentNode);
                candidates.push(grandParentNode);
            }
            var contentScore = 0;
            contentScore += 1;
            contentScore += innerText.split(',').length;
            contentScore += Math.min(Math.floor(innerText.length / 100), 3);
            parentNode.readability.contentScore += contentScore;
            if (grandParentNode) {
                grandParentNode.readability.contentScore += contentScore / 2;
            }
        }
        var topCandidate = null;
        for (var c = 0, cl = candidates.length; c < cl; c += 1) {
            candidates[c].readability.contentScore = candidates[c].readability.contentScore * (1 - getLinkDensity(candidates[c]));
            if (!topCandidate || candidates[c].readability.contentScore > topCandidate.readability.contentScore) {
                topCandidate = candidates[c];
            }
        }
        if (topCandidate === null || topCandidate.tagName === "BODY") {
            topCandidate = document.createElement("DIV");
            topCandidate.innerHTML = page.innerHTML;
            page.innerHTML = "";
            page.appendChild(topCandidate);
            initializeNode(topCandidate);
        }
        //SAMSUNG CHANGE >> MPSG5897 PART2 - Now we call the SearchLevelHigher implementation so that we search for
        //potential image additions within the siblingNodes to the topCandidate's parent and grandParentNodes. The variable 'Levels'
        //denotes how high up the DOM tree we want to search for the image node - 0 means only the siblingNodes of the
        //topCandidate.parentNode and 1 means the topCandidate.parentNode.ChildNodes and topCandidate.grandParentNode.ChildNodes
        //in the future, if on any sites, we still find that images above the readable Article are not getting displayed, then we need 
        //to pass the desired value of 'n' to decide how high up the DOM Tree our reverse-BFS should search upto to add images.
        var articleContent = SearchLevelHigher(1, topCandidate, page);
        //SAMSUNG CHANGE << MPSG5897 PART2
        prepArticle(articleContent);
        for (var pt = 0; pt < nodesToScore.length; pt += 1) {
            var parentNode = nodesToScore[pt].parentNode;
            var grandParentNode = parentNode ? parentNode.parentNode : null;
            if (parentNode) { // SAMSUNG CHANGE - MPSG5740
                delete parentNode.readability;
            }
            if (grandParentNode) {
                delete grandParentNode.readability;
            }
        }
        if (articleContent.innerHTML == '') {
            return null;
        }
        return articleContent.innerHTML;
    }
    //SAMSUNG CHANGE - MPSG5897 PART 2>>

    function SearchLevelHigher(Levels, topCandidate, page) {
        var articleContent = document.createElement("DIV");
        articleContent.id = "readability-content";
        var topCandidateCheckNode = topCandidate;
        for (var x = -1; x < Levels; x++) {
            var siblingScoreThreshold = Math.max(10, topCandidate.readability.contentScore * 0.2);
            var siblingNodes = topCandidateCheckNode.parentNode ? topCandidateCheckNode.parentNode.childNodes : null;
            var otherElementImageSearchToProceed = true; //SAMSUNG CHANGE - MPSG5897 PART2 - so we only search above the topCandidate
            for (var s = 0; siblingNodes && s < siblingNodes.length; s += 1) {
                var siblingNode = siblingNodes[s];
                var append = false;
                if (!siblingNode) {
                    continue;
                }
                if (siblingNode === topCandidate) {
                    append = true;
                    otherElementImageSearchToProceed = false; //SAMSUNG CHANGE - MPSG5897 PART2
                } else if (siblingNode === topCandidateCheckNode)
                otherElementImageSearchToProceed = false;
                var contentBonus = 0;
                if (siblingNode.className === topCandidate.className && topCandidate.className !== "") {
                    contentBonus += topCandidate.readability.contentScore * 0.2;
                }
                //SAMSUNG CHANGE MPSG5897 - PART2 >> - Since we have already identified the topCandidate, we should only append to it
                //if the siblingNode has the same className as the topCandidate node which means that the siblingNode has content
                //that would normally be displayed on the page as beginning/belonging to the topCandidate block
                if (typeof siblingNode.readability !== 'undefined' && topCandidate.className !== "" && siblingNode.className === topCandidate.className && (siblingNode.readability.contentScore + contentBonus) >= siblingScoreThreshold) {
                    //SAMSUNG CHANGE MPSG5897 - PART 2<<
                    append = true;
                }
                //SAMSUNG CHANGE MPSG5897 - PART2>>
                if (siblingNode.nodeName === "P" && topCandidate.className !== "" && siblingNode.className === topCandidate.className) {
                    //SAMSUNG CHANGE MPSG5897 - PART 2<<
                    var linkDensity = getLinkDensity(siblingNode);
                    var nodeContent = getInnerText(siblingNode);
                    var nodeLength = nodeContent.length;
                    if (nodeLength > 80 && linkDensity < 0.25) {
                        append = true;
                    } else if (nodeLength < 80 && linkDensity === 0 && nodeContent.search(/\.( |$)/) !== -1) {
                        append = true;
                    }
                }
                if (!append && otherElementImageSearchToProceed) {
                    var anyImagesToAdd = isImageWorthySiblingNode(siblingNode, page);
                    if (anyImagesToAdd !== false) {
                        if (articleContent.childElementCount === 0)
                        articleContent.appendChild(anyImagesToAdd.cloneNode(false));
                        else
                        articleContent.insertBefore(anyImagesToAdd.cloneNode(false), articleContent.childNodes[0]);
                    }
                }
                if (append) {
                    var nodeToAppend = null;
                    if (siblingNode.nodeName !== "DIV" && siblingNode.nodeName !== "P") {
                        nodeToAppend = document.createElement("DIV");
                        try {
                            nodeToAppend.id = siblingNode.id;
                            nodeToAppend.innerHTML = siblingNode.innerHTML;
                        } catch(er) {
                            nodeToAppend = siblingNode;
                            s -= 1;
                            sl -= 1;
                        }
                    } else {
                        nodeToAppend = siblingNode;
                    }
                    articleContent.appendChild(nodeToAppend.cloneNode(true));
                }
            }
            topCandidateCheckNode = topCandidateCheckNode.parentNode;
        }
        return articleContent;
    }
    //SAMSUNG CHANGE - MPSG5897 PART 2<<
    //SAMSUNG CHANGE - MPSG5897 PART 2 >>

    function isImageWorthySiblingNode(node, page) {
        if (!node || node.nodeType === 3 || node.nodeType === 8)
            return false;
        var ImageContents = node.getElementsByTagName("IMG");
        var returnValue = false;
        //Key Block Start - the reason we need this block is because the cloneNode operation done earlier for the document.body means
        //that the page.contains(node) returns true while the page.ownerDocument.body.contains(node) returns false when in the
        //ideal scenario both should return true. This is needed so the getComputedStyles gets called on the correct document object
        //we cannot use the .style call directly as that is only valid for the defined element styles and not computed element styles
        //which arise from inheritance, percentage calculations etc.
        var Index = -1;
        var pageElements = page.getElementsByTagName('*');
        for (var x = 0; x < pageElements.length; x++) {
            if (pageElements[x] === node)
                Index = x;
        }
        var pageOwnerDocumentElements = page.ownerDocument.body.getElementsByTagName('*');
        //Key Block End
        var inlineIndentationFloat = GetStyle(pageOwnerDocumentElements[Index], "float");
        var inlineIndentationDisplay = GetStyle(pageOwnerDocumentElements[Index], "display");
        for (var i = 0, icl = ImageContents.length; i < icl; i++) {
            //The logic is that inline images to the left or right of the text will have area atleast 40000 else won't be displayed.
            //this eliminates spurious ads, extraneous text, image links etc.
            if (inlineIndentationFloat && (inlineIndentationFloat === "right" || inlineIndentationFloat === "left")) {
                if (ImageContents[i].width * ImageContents[i].height >= 40000)
                    returnValue = ImageContents[i];
            }
            //the logic is that a displayable image if it is not inline will be within a display block to show it above the readable text content
            else if (inlineIndentationFloat && inlineIndentationFloat === "none" && inlineIndentationDisplay === "block" && (ImageContents[i].width * ImageContents[i].height >= 15000)) {
                returnValue = ImageContents[i];
            }
        }
        return returnValue;
    }
    //SAMSUNG CHANGE - MPSG5897 - PART 2 <<
    //SAMSUNG CHANGE - MPSG5897 - PART2>>

    function GetStyle(Element, CssProperty) {
        var strValue = "";
        if (page.ownerDocument.defaultView && page.ownerDocument.defaultView.getComputedStyle)
            strValue = page.ownerDocument.defaultView.getComputedStyle(Element, "").getPropertyValue(CssProperty);
        return strValue;
    }
    //SAMSUNG CHANGE - MPSG5897 - PART 2<<

    function getArticleTitle() {
        var curTitle = "",
            origTitle = "";
        try {
            curTitle = origTitle = document.title;
            if (typeof curTitle !== "string") {
                curTitle = origTitle = getInnerText(document.getElementsByTagName('title')[0]);
            }
        } catch(e) {}
        curTitle = cleanUpTitleCandidate(curTitle);
        if (curTitle.length > 150 || curTitle.length < 15) {
            var hOnes = document.getElementsByTagName('h1');
            if (hOnes.length === 1) {
                curTitle = getInnerText(hOnes[0]);
            }
        }
        curTitle = curTitle.replace(regexps.trim, "");
        if (curTitle.split(' ').length <= 4) {
            curTitle = origTitle;
        }
        try {
            var metaTitle = null;
            var metaElements = [];
            metaElements = document.getElementsByTagName('meta');
            var resultProp = null,
                resultName = null,
                resultid = null;
            for (var i = 0, ml = metaElements.length; i < ml; i += 1) {
                if (metaElements[i]) {
                    resultProp = (metaElements[i].getAttribute('property')) ? (metaElements[i].getAttribute('property').toLowerCase().indexOf('title') !== -1) : null;
                    resultName = (metaElements[i].getAttribute('name')) ? (metaElements[i].getAttribute('name').toLowerCase().indexOf('title') !== -1) : null;
                    resultid = (metaElements[i].getAttribute('id')) ? (metaElements[i].getAttribute('id').toLowerCase().indexOf('title') !== -1) : null;
                    if (resultProp || resultName || resultid) {
                        //implies that meta title content is present
                        metaTitle = metaElements[i].getAttribute('content');
                    }
                }
            }
            if (!metaTitle)
            return curTitle;
            else {
                metaTitle = cleanUpTitleCandidate(metaTitle);
                metaTitle = metaTitle.replace(regexps.trim, "");
                return metaTitle.length > curTitle.length ? metaTitle : curTitle;
            }
        } catch (e) {
            return curTitle;
        }
    }

    function cleanUpTitleCandidate(Title) {
        var curTitle = "",
            origTitle = "";
        curTitle = origTitle = Title;
        if (curTitle.match(/ [\|\-] /)) {
            curTitle = origTitle.replace(/(.*)[\|\-] .*/gi, '$1');
            if (curTitle.split(' ').length < 3) {
                curTitle = origTitle.replace(/[^\|\-]*[\|\-](.*)/gi, '$1');
            }
        }
        if (curTitle.indexOf(': ') !== -1) {
            curTitle = origTitle.replace(/.*:(.*)/gi, '$1');
            if (curTitle.split(' ').length < 3) {
                curTitle = origTitle.replace(/[^:]*[:](.*)/gi, '$1');
            }
        }
        return curTitle;
    }

    function initReader() {
        var article_block = grabArticle();
        if (article_block) {
            var reader_header = getArticleTitle();
            var reader_string = "<style type='text/css'>body {background-color:rgba(232,232,232,1);}.SISO_page {border: 4px solid #c4d1d0; border-color: black transparent transparent transparent; margin:10px 0px 10px 2px;padding:2px;word-wrap:break-word; text-align:justify;}.SISO_page-separator {font-size:10px;text-align:right;width:95%;color:#94acaa;padding-top:10px}.SISO_page-num {font-size:10px;color:#94acaa}.SISO_page-total {font-size:10px;color:#94acaa}.SISO_header {font-size: 1.5em; margin:10px 0px 10px 2px;padding:2px;word-wrap:break-word; text-align:justify;font-weight: bold;} h1 { font-size: 1.17em; margin: .83em 0 } h2 { font-size: 1.17em; margin: .83em 0 } h3 { font-size: 1.0em; margin: 1.5em 0 } h4 { font-size: .83em; margin: 1.67em 0 } h5 { font-size: .75em; margin: 1.67em 0 } h6 { font-size: .75em; margin: 1.67em 0 } img { max-width: 300; }</style><meta name=\"viewport\" content=\"width=0, initial-scale=1.0, maximum-scale=2.0, minimum-scale=1.0, user-scalable=no, target-densitydpi=medium-dpi\"><div id='article_header' class='SISO_header'>" + reader_header + "</div><div id='readability-page-1' class='SISO_page'><p class='SISO_page-separator' title='No Page'></p>" + article_block + "</div>";
            return reader_string;
        } else {
            return "undefined";
        }
    }
    initReader();
} catch(e) {
    //SAMSUNG CHANGE - MPSG5927>> - Removing logging as per HQ request for security reasons
    //console.log("Reader Error - Reader.js");
    //SAMSUNG CHANGE - MPSG5927<<
}