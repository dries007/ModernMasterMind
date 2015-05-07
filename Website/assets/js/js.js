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

/**
 * Menu structure
 */
$(function() {
    /** MAIN MENU */
    var list = $("header").find("#menu");
    if (list == null) return; // No menu

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

    for (var i = 0; i < pages.length; i++)
    {
        var page = pages[i];
        var listItem = $("<li>");
        var link = $("<a>");
        list.append(listItem.append(link));

        link.attr("href", page.link);
        if (window.location.pathname.indexOf(page.link) != -1)
        {
            listItem.addClass("active");
        }
        else
        {
            listItem.addClass("inactive");
        }
        if (typeof page.icon != "undefined")
        {
            var icon = $("<i>");
            icon.addClass("fa").addClass("fa-" + page.icon).addClass("fa-lg");
            link.append(icon, "&nbsp;&nbsp;");
        }
        link.append($("<span>").text(page.name));
    }

    /** SIDE MENU */
    var subElement = $("#submenu");
    var subs = $("#content").find(".sub");
    for (var i = 0; i < subs.length; i++)
    {
        var item = $("<li>" + subs[i].innerHTML + "</li>");
        item.attr("for", subs[i].id);
        if (subs[i].tagName.toLowerCase() == "h2") item.css("text-indent", "5px");
        else if (subs[i].tagName.toLowerCase() == "h3") item.css("text-indent", "10px");
        item.click(function () { window.location.hash = "#" + this.getAttribute("for"); });
        subElement.append(item);
    }
});

/**
 * Dynamically add up to 150px padding to the #container element
 * Make the #content height perfect
 */
function setSizesAndPadding()
{
    var container = $("#container");
    if (container == null) return;

    var whitespace = $(window).width() - container.width();

    whitespace = Math.floor(whitespace / 2);
    whitespace = Math.min(whitespace, 150);

    if (whitespace != 150) $("#submenu").css("display", "none");
    else $("#submenu").css("display", "inherit");

    if (whitespace > 0) container.css("padding", "0 " + whitespace + "px");
    else container.css("padding", "0");

    // - 2 to make up for BS inconsistencies in chrome (and/or others?)
    $("#content").innerHeight(container.height() - $("header").outerHeight() - $("footer").outerHeight() - 10);
}

/**
 * Call the function on load and on window resize event
 */
$(setSizesAndPadding);
$(window).resize(setSizesAndPadding);

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

    var me = this;

    this.form.submit(function (event)
    {
        var errors = false;
        me.inputs.each(function (index, element)
        {
            element = $(this);
			
			switch (element.attr("type"))
            {
                case "numer":
                    errors = setState(errors, element, parseInt(element.val()) === Number.NaN);
                    break;
                case "email":
                    errors = setState(errors, element, element.val().indexOf('@') === -1);
                    break;
                default:
                    errors = setState(errors, element, element.val().trim().length === 0);
            }
			
			if (element.is(me.sillyQuestionElement)) 
			{
				errors = setState(errors, element, parseInt(element.val()) !== me.answer);
			}
        });
        if (errors) event.preventDefault();
        return !errors;
    });

    if (sillyQuestionId)
    {
        this.sillyQuestionElement = this.form.find("#" + sillyQuestionId);

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
 * The first title and image should be pre-loaded.
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
    this.h1 = this.element.find("h1");
    this.img = this.element.find("img");
    this.control = this.element.find(".fa");
    this.titles = titles;
    this.images = images;
    if (images.length != titles.length) console.log("Images and Titles of slideshow " + elementId + " have different lengths?")
    this.count = images.length;
    this.currentImage = 0;
    this.fadetime = fadeTime;
    this.intervalTime = intervalTime;

    var slideshow = this;
    this.control.on("click", function(e)
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