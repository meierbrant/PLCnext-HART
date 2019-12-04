requirejs.config({
    paths: {
        jquery: 'lib/jquery.min',
        knockout: 'lib/knockout.min',
        text: 'lib/text'
    }
});

requirejs(['jquery', 'knockout', '../components/all'], function($, ko) {
    $.get("http://localhost:5900/info", {}, function(data) {
        console.log(data);
    });

    ko.applyBindings();
});