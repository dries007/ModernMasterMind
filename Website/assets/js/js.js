/**
     The MIT License (MIT)

     Copyright (c) 2015 Dries007

     Made for/in relation to an education at Thomas More Mechelen-Antwerpen vzw
     Campus De Nayer - Professionele bachelor elektronica-ict

     Permission is hereby granted, free of charge, to any person obtaining a copy
     of this software and associated documentation files (the "Software"), to deal
     in the Software without restriction, including without limitation the rights
     to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
     copies of the Software, and to permit persons to whom the Software is
     furnished to do so, subject to the following conditions:

     The above copyright notice and this permission notice shall be included in all
     copies or substantial portions of the Software.

     THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
     IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
     FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
     AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
     LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
     OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
     SOFTWARE.
 */
// <!--
/**
 * Menu structure
 * jQuery for the win!
 */
$(function() {
    /** MAIN MENU */
    var list = $("header").find("#menu");
    if (list == null) return; // No menu

    // Menu data
    // Info about icons: http://fortawesome.github.io/Font-Awesome/icons/
    var pages = [
        {
            name: "Overzichtspagina",
            link: "index.html",
            icon: "newspaper-o"
        },
        {
            name: "Home",
            link: "home2.html",
            icon: "home"
        },
        {
            name: "Hardware",
            link: "hardware.html",
            icon: "cogs"
        },
        {
            name: "Software",
            link: "software.html",
            icon: "code"
        },
        {
            name: "Contact",
            link: "contact.html",
            icon: "envelope"
        }
    ];

    // Main menu tabs
    for (var i = 0; i < pages.length; i++)
    {
        var page = pages[i];
        var listItem = $("<li>");
        var link = $("<a>");
        list.append(listItem.append(link)); // Inception! (<a> in <li>)

        link.attr("href", page.link);
        if (window.location.pathname.indexOf(page.link) != -1) // url contains page name => we are here.
        {
            listItem.addClass("active");
        }
        else
        {
            listItem.addClass("inactive");
        }
        if (typeof page.icon != "undefined") // icon present in data set
        {
            var icon = $("<i>"); // FA uses <i> tags for there
            icon.addClass("fa").addClass("fa-" + page.icon).addClass("fa-lg"); // class = "fa fa-ICON fa-lg"
            link.append(icon, "&nbsp;&nbsp;"); // 2 non breaking spaces
        }
        link.append($("<span>").text(page.name)); // text in span
    }

    /** SIDE MENU */
    var subElement = $("#submenu");
    if (subElement.size() != 0)// not always there
    {
        var subs = $("#content").find(".sub"); // helper class for this purpose
        for (var i = 0; i < subs.length; i++)
        {
            var item = $("<li>" + subs[i].innerHTML + "</li>"); // use same text, but strip out outer tags
            item.attr("for", subs[i].id); // set 'for' attribute, to keep the id handy
            if (subs[i].tagName.toLowerCase() == "h2") item.css("text-indent", "5px"); // text indentation on the list item, dependant on the h level
            else if (subs[i].tagName.toLowerCase() == "h3") item.css("text-indent", "10px");
            else if (subs[i].tagName.toLowerCase() == "h4") item.css("text-indent", "15px");
            item.click(function () // the onclick magic scrolling
            {
                // Use browser magic to scroll to page section, with the ID we saved earlier.
                // Disadvantages: No smooth scrolling, clips the text right against the top of the content box.
                window.location.hash = "#" + this.getAttribute("for");
            });
            subElement.append(item); // actually add the element
        }
    }
});

/**
 * Dynamically add up to 150px padding to the #container element
 * Make the #content height perfect
 *
 * Look mom, no jQuery!
 */
function setSizesAndPadding()
{
    var container = document.getElementById("container");
    if (container == null) return; // Not there on index page

    //              browser screen width - css value for the #container element (even if overwritten anywhere else in css) ONLY WITH PIXELS, NOT PERCENTAGES!
    var whitespace = window.innerWidth - window.getComputedStyle(container).getPropertyValue("width").replace("px", "");

    // divide by 2 and make sure its maximum 150px
    whitespace = Math.floor(whitespace / 2);
    whitespace = Math.min(whitespace, 150);

    var submenu = document.getElementById("submenu");
    if (submenu != null) // not always there...
    {
        if (whitespace != 150) submenu.style.display = "none"; // hide, don't remove. Gets rechecked if page is resized
        else submenu.style.display = "inherit"; // Show
    }

    if (whitespace > 0) container.style.padding = "0 " + whitespace + "px"; // add sidebar with responsive width to avoid useless scrollbars
    else container.style.padding = "0";

    // - 10 to make up for BS inconsistencies in chrome (and/or others?) that appear on page reload and other stuff
    // sets the #content height to exactly match the browser height - (header and footer), since we want the footer to stick to the bottom
    // Stops working when the browser height is too small for the header & footer
    document.getElementById("content").style.height = container.offsetHeight - document.getElementsByTagName("header")[0].offsetHeight - document.getElementsByTagName("footer")[0].offsetHeight - 10 + "px";
}

/**
 * To show that its possible without jQuery...
 * Call the function on window resize event & on load
 */
function addEventHandler(elem, type, eventHandle)
{
    if (elem.addEventListener) //W3C DOM spec method
    {
        elem.addEventListener(type, eventHandle, false);
    }
    else if (elem.attachEvent) // IE < 11 bullshit
    {
        elem.attachEvent("on" + type, eventHandle);
    }
    else // Last resort!
    {
        elem["on"+type] = eventHandle;
    }
}
addEventHandler(window, "resize", setSizesAndPadding); // on resize
addEventHandler(window, "load", setSizesAndPadding); //on load

/**
 * ========================================== FORM ==========================================
 *
 * All input and textareas get checked based on there type, if type is text, the field can't be empty.
 * A Form must have:
 *  - All textboxes & textares must have class "input"
 *  - sillyQuestion, if any, must have a label
 *
 * @param formId
 * @param sillyQuestionId Simple math question, can be null
 * @constructor
 */
function Form(formId, sillyQuestionId)
{
    this.form = $("#" + formId);
    this.inputs = this.form.find(".input");

    var me = this; // for later reference

    this.form.submit(function (event) // on submit
    {
        var errors = false;
        me.inputs.each(function (index, element) // for each input field
        {
            element = $(this); // get the element as a jQuery object
			
			switch (element.attr("type")) // how to verify depends on the type
            {
                case "numer":   // integer check, we don't need doubles
                    errors = setState(errors, element, parseInt(element.val()) === Number.NaN);
                    break;
                case "email":   // Über basic email check, a modern browser has a better one anyways
                    errors = setState(errors, element, element.val().indexOf('@') === -1);
                    break;
                default:        // For any other type, just make sure that there is something in there.
                    errors = setState(errors, element, element.val().trim().length === 0);
            }

			if (element.is(me.sillyQuestionElement)) // if the current element is the sillyQuestionElement
			{
                // Check the answer AFTER the regular checks, because this one is more important
				errors = setState(errors, element, parseInt(element.val()) !== me.answer);
			}
        });
        // prevent submit
        if (errors) event.preventDefault();
        return !errors;
    });

    if (sillyQuestionId) // if there is a sillyQuestionId
    {
        this.sillyQuestionElement = this.form.find("#" + sillyQuestionId); // get and save the element

        // Make up some math
        var i1 = 1 + Math.floor(Math.random() * 5);
        var i2 = 1 + Math.floor(Math.random() * 5);

        this.answer = i1 + i2;
        this.form.find("label[for=" + sillyQuestionId + "]").text(i1 + " + " + i2 + " =");
    }
}

/**
 * Helper function for Form
 * Adds "error" class if error indicator is true, otherwise removes "error" class
 *
 * @param errors Old error value
 * @param element jQuery element
 * @param b Error indicator
 * @returns New Error value (errors || b) {boolean}
 */
function setState(errors, element, b)
{
    if (b) element.addClass("error");
    else element.removeClass("error");
    return errors || b;
}

/**
 * ========================================== SLIDESHOW ==========================================
 *
 * Calls start function automatically.
 * A slideshow must have:
 *  - Outer element with id 'elementId'
 *  - 1 Title element in tag 'h1'
 *  - 1 Image element in tag 'img'
 *  - 1 I element with FontAwesome class "fa fa-pause"
 * ! The first title and image should be pre-loaded !
 *
 * @param elementId Id without #
 * @param titles Array of Strings
 * @param images Array of Strings
 * @param intervalTime Time in ms
 * @param fadeTime Time in ms
 * @constructor
 */
function Slideshow(elementId, titles, images, intervalTime, fadeTime)
{
    this.element = $("#" + elementId);
    this.h1 = this.element.find("h1"); // used as title
    this.img = this.element.find("img"); // actual image
    this.control = this.element.find(".fa"); // play/pause button
    this.titles = titles; // data
    this.images = images; // data
    if (images.length != titles.length) // Sanity check
    {
        console.log("Images and Titles of slideshow " + elementId + " have different lengths?")
    }
    this.count = images.length;
    this.currentImage = 0; // Start with 0 (pre loaded in the HTML)
    this.fadetime = fadeTime;
    this.intervalTime = intervalTime;

    var slideshow = this;
    this.control.on("click", function(e) // pause/play
    {
        slideshow.toggle();
    });

    /*
     * Make sure 'this' is always the slideshow object
     */
    this.start = this.start.bind(this);
    this.stop = this.stop.bind(this);
    this.toggle = this.toggle.bind(this);
    this.cycle = this.cycle.bind(this);

    this.start();
}

// Uses prototypes here to avoid creating more then one instance of the functions.
// Not really necessary, but better coding standard.

/**
 * Function to start the slideshow
 * Must be bound!
 */
Slideshow.prototype.start = function ()
{
    if (this.interval == null)
    {
        this.interval = setInterval(this.cycle, this.intervalTime);

        this.control.removeClass("fa-play");
        this.control.addClass("fa-pause");
    }
};

/**
 * Function to stop the slideshow
 * Must be bound!
 */
Slideshow.prototype.stop = function ()
{
    if (this.interval != null)
    {
        clearInterval(this.interval);
        this.interval = null;

        this.control.removeClass("fa-pause");
        this.control.addClass("fa-play");
    }
};

/**
 * Function to toggle the slideshow
 * Must be bound!
 */
Slideshow.prototype.toggle = function ()
{
    if (this.interval != null)
    {
        this.stop();
    }
    else
    {
        this.start();
    }
};

/**
 * Function to cycle the slideshow
 * Must be bound!
 */
Slideshow.prototype.cycle = function ()
{
    var slideshow = this;

    slideshow.h1.fadeOut(slideshow.fadetime, function ()
    {
        slideshow.h1.text(slideshow.titles[slideshow.currentImage]);
        slideshow.h1.fadeIn(slideshow.fadetime);
    });

    slideshow.img.fadeOut(500, function ()
    {
        slideshow.img.attr("src", slideshow.images[slideshow.currentImage]);
        slideshow.img.fadeIn(500);
    });

    slideshow.currentImage++;
    if (slideshow.currentImage == slideshow.count) slideshow.currentImage = 0;
};
// -->
