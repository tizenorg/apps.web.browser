//var currentDiv=null;
//var lineElementsNo = 3;

function getCurrent(){
    var currentDiv = $("div.selected");
    if(currentDiv.length == 0 ){
        currentDiv = $("div.bookmark").first().addClass("selected");
        autoScroll(currentDiv);
        return 0;
    }
    return currentDiv;
}
Mousetrap.bind('left', function(e){
    var currentDiv = getCurrent();
    if(currentDiv){
        var testPrev = currentDiv.prev();
        if(testPrev.length && testPrev.is("div.bookmark")){
            currentDiv.removeClass("selected");
            testPrev.addClass("selected");
            autoScroll(testPrev);
        }
    }
});

Mousetrap.bind('right', function(e){
    var currentDiv = getCurrent();
    if(currentDiv){
        var testNext = currentDiv.next();
        if( testNext.length && testNext.is("div.bookmark") ){
            currentDiv.removeClass("selected");
            testNext.addClass("selected");
            autoScroll(testNext);
        }
    }
});

Mousetrap.bind('down', function(e){
    var currentDiv = getCurrent();
    if(currentDiv){
        var testNext = currentDiv;
        var rowLen=getElementsInRow(currentDiv);
        for(var i = 0; i<rowLen; i++){
            if(testNext.next().length != 0 && testNext.next().is("div.bookmark")){
                testNext = testNext.next();
            }
        }
        currentDiv.removeClass("selected");
        testNext.addClass("selected");
        autoScroll(testNext);
    }
});

Mousetrap.bind('up', function(e){
    var currentDiv = getCurrent();
    if(currentDiv){
        var testNext = currentDiv;
        var leftNo = getLeftElements(currentDiv);
        for(var i = 0; i <= leftNo; i++){
            testNext = testNext.prev();
        }
        testNext = getFirstInLine(testNext);
        for(var i = 0; i < leftNo; i++){
            testNext = testNext.next();
        }
        currentDiv.removeClass("selected");
        testNext.addClass("selected");
        autoScroll(testNext);
//         testNext.localScroll();
    }
});

Mousetrap.bind('enter', function(e){
    var currentDiv = $("div.selected");
    if(currentDiv.length == 1){
        var anchor = currentDiv.find("a");
        if(anchor.length == 1){
            window.location = anchor.attr("href");
        }
        return;
    }
});

function getElementsInRow(source){
    return getRightElements(source) + getLeftElements(source);
};

function getRightElements(source){
    var value = 1;
    if(source.next().length > 0 && source.next().is("div.bookmark")){
        if(source.position().top == source.next().position().top){
            value = 1 + getRightElements(source.next());
        }
    }
    return value;
}

function getLeftElements(source){
    var value = 0;
    if(source.prev().length > 0 && source.prev().is("div.bookmark")){
        if(source.position().top == source.prev().position().top){
            value = 1 + getLeftElements(source.prev());
        }
    }
    return value;
}

function getFirstInLine(source){
    var prev = source.prev();
    if(prev.length && (prev.position().top == source.position().top)){
        return getFirstInLine(prev);
    }
    return source;
}

function autoScroll(source){
    $('html, body').animate({scrollTop: source.offset().top - 110}, 500);
}

