requirejs.config({
    paths: {
        jquery: 'lib/jquery.min',
        knockout: 'lib/knockout.min',
        knockstrap: 'lib/knockstrap.min',
        bootstrap: 'lib/bootstrap-3.4.1/bootstrap.min',
        text: 'lib/text'
    },
    shim: {
        bootstrap: {
            deps: ['jquery']
        },
        knockstrap: {
            deps: ['knockout', 'jquery', 'bootstrap']
        }
    }
});

requirejs(['jquery', 'knockout', 'knockstrap', 'bootstrap', '../components/all'], function($, ko) {
    $.get("http://localhost:5900/info", {}, function(data) {
        console.log(data);
    });

    ko.applyBindings();
});