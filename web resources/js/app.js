($(function () {
    var timersTemplateTemplate = '<div class=\"add-button-container\"><button id=\"add-new\" class=\"btn btn-large btn-primary\">Add new<\/button><\/div><div class=\"page-header\"><h1 class=\"title\">Timers<\/h1><\/div><div id=\"addModal\" class=\"modal hide fade\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"myModalLabel\" aria-hidden=\"true\"> <div class=\"modal-header\"> <button type=\"button\" class=\"close\" data-dismiss=\"modal\" aria-hidden=\"true\">X<\/button> <h3 id=\"myModalLabel\">Add \/ edit timer<\/h3> <\/div> <div class=\"modal-body\"> <form id=\"addTimer\" class=\"form-horizontal\" action=\"#\"> <div class=\"control-group\"> <label class=\"control-label\" for=\"name\">Name:<\/label> <div class=\"controls\"> <input type=\"text\" id=\"name\" name=\"name\"\/> <\/div> <\/div> <div class=\"control-group\"> <label class=\"control-label\">Action:<\/label> <div class=\"controls\"> <label class=\"radio\"> <input type=\"radio\" name=\"action\" value=\"0\"> On <\/label> <label class=\"radio\"> <input type=\"radio\" name=\"action\" value=\"1\"> Off <\/label> <\/div> <\/div> <div class=\"control-group\"> <label class=\"control-label\" for=\"device\">Device:<\/label> <div class=\"controls\"> <select id=\"device\" name=\"did\"><option value=\"0\">Not Set<\/option></option> <% for (var j = 0; j < data.length; j++) { %> <option value=\"<% print(data[j].id) %>\"><% print(data[j].name) %><\/option> <% } %> <\/select> <\/div> <\/div> <div class=\"control-group\"> <label class=\"control-label\" for=\"name\">Time:<\/label> <div class=\"controls\"> <div class=\"input-append bootstrap-timepicker-component\"> <input type=\"text\" id=\"daytime\" name=\"daytime\" class=\"input-small\"> <span class=\"add-on\"> <i class=\"icon-time\"><\/i> <\/span> <\/div> <\/div> <\/div> <div class=\"control-group\"> <label class=\"control-label\">Repeat<\/label> <div class=\"controls\"> <label class=\"radio\"> <input type=\"radio\" name=\"repeat\" id=\"repeat1\" value=\"1\"> Every day <\/label> <label class=\"radio\"> <input type=\"radio\" name=\"repeat\" id=\"repeat2\" value=\"0\"> <div class=\"input-append date\" id=\"date\" data-date-format=\"mm-dd-yyyy\"> <input class=\"span2\" size=\"16\" type=\"text\" name=\"date\"><span class=\"add-on\"><i class=\"icon-th\"><\/i><\/span> <\/div> <\/label> <\/div> <\/div> <div class=\"control-group\"> <label class=\"control-label\">Status:<\/label> <div class=\"controls\"> <label class=\"radio\"> <input type=\"radio\" name=\"state\" value=\"0\"> Pending <\/label> <label class=\"radio\"> <input type=\"radio\" name=\"state\" value=\"1\"> Fired  <\/label> <\/div> <\/div><\/form> <\/div> <div class=\"modal-footer\"> <button class=\"btn\" data-dismiss=\"modal\" aria-hidden=\"true\">Cancel<\/button> <button class=\"btn btn-primary\" data-dismiss=\"modal\" id=\"save-new\">Save<\/button> <\/div> <\/div> <div class=\"row\" id=\"timers\"> <\/div>  ';
    var deviceTemplateTemplate = ' <div class=\"device-container"><div class=\"control-buttons\"> <button class=\"delete btn btn-small\">Delete<\/button> <button class=\"edit btn btn-small\">Edit<\/button> <\/div> <h1><%= name %><\/h1> <br> <button class=\"toggle btn btn-large <% if (state){ print(\"btn-primary\"); } %>\"><% if (state){ print(\"On\"); } else{ print(\"Off\"); } %> <\/button><\/div>';
    var devicesTemplateTemplate = '<div class=\"page-header\"><h1 class=\"title\">Devices<\/h1><\/div><div id=\"addDeviceModal\" class=\"modal hide fade\" tabindex=\"-1\" role=\"dialog\" aria-labelledby=\"myModalLabel\" aria-hidden=\"true\"> <div class=\"modal-header\"> <button type=\"button\" class=\"close\" data-dismiss=\"modal\" aria-hidden=\"true\">X<\/button> <h3 id=\"deviceModalLabel\">Edit device<\/h3> <\/div> <div class=\"modal-body\"> <form id=\"addDevice\" class=\"form-horizontal\" action=\"#\"> <div class=\"control-group\"> <label class=\"control-label\" for=\"name\">Name:<\/label> <div class=\"controls\"> <input type=\"text\" id=\"name\" name=\"name\"\/> <\/div> <\/div> <\/form> <\/div> <div class=\"modal-footer\"> <button class=\"btn\" data-dismiss=\"modal\" aria-hidden=\"true\">Cancel<\/button> <button class=\"btn btn-primary\" data-dismiss=\"modal\" id=\"save-new-device\">Save<\/button> <\/div> <\/div> <div class=\"row\" id=\"devices\"> <\/div>  ';
    var timerTemplateTemplate = '  <div class=\"timer-container"><div class=\"control-buttons\"><button class=\"delete btn btn-small\">Delete<\/button> <button class=\"edit btn btn-small\">Edit<\/button> <\/div> <h1><%= name %><\/h1> Device: <%= devicename %><br> <% var date = new Date(time*1000 ); date = new Date(date.getTime() + date.getTimezoneOffset() * 60000); if (time > 86400000){ print(date); } else{ var hours = Math.floor(time/3600000); var minutes = (time - (hours * 3600000))/60000; print(\"Every day: \"+ hours + \':\' + (\"0\" + minutes).substr(-2)); } %>  <\/div>';

    var TimerModel = Backbone.Model.extend({
        defaults:{
            action:0,
            date:"",
            daytime:"",
            devicename:"",
            did:0,
            name:"New Timer",
            repeat:1,
            state:0,
            time:0
        },
        urlRoot:"timers"
    });

    var TimerCollection = Backbone.Collection.extend({
        model:TimerModel,
        url:"timers"
    });

    var DeviceModel = Backbone.Model.extend({
        defaults:{
            name:"No Name",
            state:0
        },
        urlRoot:"devices"
    });

    var DeviceCollection = Backbone.Collection.extend({
        model:DeviceModel,
        url:"devices"
    });

    var TimerModelView = Backbone.View.extend({
        tagName:"div",
        className:"span6",
        template:_.template(timerTemplateTemplate),
        modal:null,

        render:function () {
            this.$el.html(this.template(this.model.toJSON()));
            return this;
        },

        events:{
            "click .delete":"deleteTimer",
            "click .edit":"editTimer"
        },

        deleteTimer:function () {
            this.model.destroy();
            this.remove();
        },

        editTimer:function (e) {
            this.trigger('edit', e, this.model);
        }
    });

    var TimerCollectionView = Backbone.View.extend({

        template:null,
        devices:null,
        collection:new TimerCollection(),

        initialize:function () {
            this.collection.fetch();
            var deviceCollection = new DeviceCollection();

            var that = this;
            deviceCollection.fetch({success:function () {
                that.devices = deviceCollection.toJSON();
                that.template = _.template(timersTemplateTemplate, {data:that.devices});
                that.modelBinder = new Backbone.ModelBinder();
                that.$el.html(that.template);
                that.render();
                that.collection.on("reset", that.render, that);
                that.collection.on("add", that.renderTimer, that);
                that.$('#daytime').timepicker({
                    showSeconds:false,
                    showMeridian:false,
                    minuteStep:1
                });
                that.$("#date").datepicker({autoclose:true});
            }
            });
        },

        render:function () {
            this.$el.find(".timer-container").remove();

            _.each(this.collection.models, function (item) {
                this.renderTimer(item);
            }, this);
        },

        renderTimer:function (item) {

            for (var j = 0; j < this.devices.length; j++) {

                if (this.devices[j].id === item.get("did")) {
                    item.set("devicename", this.devices[j].name);
                    break;
                }
            }

            var timerView = new TimerModelView({
                model:item
            });

            var that = this;

            timerView.on('edit', function (e, model) {

                that.showDialog(e, model);
            });
            this.$('#timers').prepend(timerView.render().el);
        },

        events:{
            "click #save-new":"saveEdits",
            "click #add-new":"showDialog"
        },

        showDialog:function (e, model) {
            var that = this;

            if (typeof model === 'undefined') {
                this.formModel = new TimerModel();
            }
            else {
                this.formModel = model;
            }

            var date = new Date();

            if (this.formModel.get("time") < 86400000) {
                if (this.formModel.get("time") != 0) {
                    var hours = Math.floor(this.formModel.get("time") / 3600000);
                    var minutes = (this.formModel.get("time") - (hours * 3600000)) / 60000;

                    date.setHours(hours);
                    date.setMinutes(minutes);
                }
                this.formModel.set("repeat", 1);
            }
            else {
                date = new Date(model.get("time") * 1000 + (date.getTimezoneOffset() * 60000));
                this.formModel.set("repeat", 0);
            }

            this.formModel.set("date", ("0" + (date.getMonth() + 1)).substr(-2) + '-' + ("0" + date.getDate()).substr(-2) + '-' + date.getFullYear());
            this.formModel.set("daytime", date.getHours() + ':' + ("0" + date.getMinutes()).substr(-2));

            for (var j = 0; j < this.devices.length; j++) {
                if (this.devices[j].id == this.formModel.get("did")) {
                    this.formModel.set("devicename", this.devices[j].name);
                }
            }

            var numberConverter = function (direction, value) {
                if (direction === 'ModelToView') {
                    return "" + value;
                }

                if (direction === 'ViewToModel') {
                    return Number(value);
                }
            };

            var didConverter = function (direction, value, attribute, model) {
                if (direction === 'ModelToView') {
                    return "" + value;
                }

                if (direction === 'ViewToModel') {
                    for (var j = 0; j < that.devices.length; j++) {

                        if (that.devices[j].id == model.get("did")) {
                            model.set("devicename", that.devices[j].name);
                        }
                    }
                    return Number(value);
                }
            };

            var bindings = {
                action:{selector:'[name=action]', converter:numberConverter},
                daytime:{selector:'[name=daytime]'},
                name:{selector:'[name=name]'},
                state:{selector:'[name=state]', converter:numberConverter},
                did:{selector:'[name=did]', converter:didConverter},
                repeat:{selector:'[name=repeat]', converter:numberConverter},
                date:{selector:'[name=date]'}
            };

            this.modelBinder.bind(this.formModel, this.$('#addTimer'), bindings);

            $('#addModal').modal('show');
        },

        saveEdits:function () {
            var hours = Number(this.formModel.get("daytime").split(":")[0]);
            var minutes = Number(this.formModel.get("daytime").split(":")[1]);

            if (this.formModel.get("repeat") === 1) {
                this.formModel.set("time", (hours * 60 + minutes) * 60000);
            }
            else {
                var year = this.formModel.get("date").split("-")[2];
                var month = this.formModel.get("date").split("-")[0];
                var day = this.formModel.get("date").split("-")[1];
                var date = new Date(year, month - 1, day, hours, minutes);
                this.formModel.set("time", (date.getTime() - (date.getTimezoneOffset() * 60000)) / 1000);
            }

            this.formModel.unset("daytime", { silent:true });
            this.formModel.unset("repeat", { silent:true });
            this.formModel.unset("date", { silent:true });

            if (this.formModel.isNew()) {
                this.collection.add(this.formModel);
            }

            var deviceName = this.formModel.get("devicename");

            this.formModel.unset("devicename", { silent:true });
            this.formModel.save();
            this.formModel.set("devicename", deviceName);

            this.render();
        }
    });

    var DeviceModelView = Backbone.View.extend({

        tagName:"div",
        className:"span6",
        template:_.template(deviceTemplateTemplate),
        modal:null,

        render:function () {
            this.$el.html(this.template(this.model.toJSON()));
            return this;
        },

        events:{
            "click .delete":"deleteDevice",
            "click .edit":"editDevice",
            "click .toggle":"toggleSwitch"
        },

        toggleSwitch:function () {
            this.model.set("state", Number(!this.model.get("state")));
            this.model.save();

            if (this.model.get("state") === 0) {
                this.$('.toggle').removeClass("btn-primary");
                this.$('.toggle').html("Off");
            }
            else {
                this.$('.toggle').addClass("btn-primary");
                this.$('.toggle').html("On");
            }
        },

        deleteDevice:function () {
            this.model.destroy();
            this.remove();
        },

        editDevice:function (e) {
            this.trigger('edit', e, this.model);
        }

    });


    var DeviceCollectionView = Backbone.View.extend({
        template:_.template(devicesTemplateTemplate),
        collection:new DeviceCollection(),

        initialize:function () {
            this.collection.fetch();
            this.modelBinder = new Backbone.ModelBinder();
            this.$el.html(this.template());
            this.render();
            this.collection.on("reset", this.render, this);
            this.collection.on("add", this.renderDevice, this);
        },

        render:function () {
            this.$el.find(".device-container").remove();
            _.each(this.collection.models, function (item) {
                this.renderDevice(item);
            }, this);
        },

        renderDevice:function (item) {
            var deviceView = new DeviceModelView({
                model:item
            });
            var that = this;
            deviceView.on('edit', function (e, model) {
                that.showDialog(e, model);
            });
            this.$('#devices').prepend(deviceView.render().el);
        },


        events:{
            "click #save-new-device":"saveDevice"
        },

        showDialog:function (e, model) {
            if (typeof model === 'undefined') {
                this.formModel = new DeviceModel();
            }
            else {
                this.formModel = model;
            }

            var bindings = {
                name:{selector:'[name=name]'}
            };
            this.modelBinder.bind(this.formModel, this.$('#addDevice'), bindings);
            $('#addDeviceModal').modal('show');
        },

        saveDevice:function () {

            if (this.formModel.isNew()) {
                this.collection.add(this.formModel);
            }

            this.formModel.save();
            this.render();

        }
    });

    var TimersRouter = Backbone.Router.extend({
        devices:new DeviceCollection(),
        timers:new TimerCollection(),

        routes:{
            "":"showDevices",
            "timers":"showTimers",
            "devices":"showDevices"
        },

        showTimers:function () {
            this.changePage(new TimerCollectionView({}).el)
        },

        showDevices:function () {
            this.changePage(new DeviceCollectionView().el);
        },

        changePage:function (page) {
            $("#main-container").html(page);
        }
    });

    var timersRouter = new TimersRouter();

    Backbone.history.start();

}));