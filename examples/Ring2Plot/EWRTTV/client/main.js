//    Earthworm Real Time Trace Viewer is a program that takes in changes from MongoDB and plots data using plotly and javascript
//    Copyright (C) 2018  Francisco J Hernandez Ramirez
//    You may contact me at FJHernandez89@gmail.com, FHernandez@boritechsolutions.com
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU Affero General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU Affero General Public License for more details.
//
//    You should have received a copy of the GNU Affero General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>

import { Template } from 'meteor/templating';
import { ReactiveVar } from 'meteor/reactive-var';
import { get_collection } from '../lib/collection_manager.js';
import '../lib/routes.js';

import './main.html';

Session.set("Sta",undefined);
Session.set("Chan",undefined);
  
Template.info.onCreated(function () {
  Session.set("Sta",undefined);
  Session.set("Chan",undefined);
});

Template.info.events({
  'submit .form-station' (event, instance) {
    event.preventDefault();
    var getStation = event.target.station.value;
    var getChannel = event.target.channel.value;
    Meteor.call('checkcol', { 
        sta: getStation, 
        chan: getChannel
      }, (err, res) => {
        if (err){
          alert(err);
        } else {
          Session.set("Sta", getStation);
          Session.set("Chan", getChannel);
          Router.go('/graph');
          //Success!
        }
    });
  }
});

Template.graph.helpers({
  foo() {
    return Session.get("Sta");
  },
  doo() {
    return Session.get("Chan");
  }
});

Template.graph.onRendered(function () {
  var sta = Session.get("Sta");
  var cha = Session.get("Chan");
  if (sta === undefined || cha === undefined){
    Router.go('/');
  }
  var data = get_collection(sta).find({'channel': cha});
  var time = [];
  var trace = [];
  /*data.forEach( function(element) {
    time = time.concat(element.times);
    trace = trace.concat(element.data);
  }); 
  time = time.map( function (element) {
    return new Date(element);
  });*/
  var ndata = [{
    x: time, 
    y: trace,
    mode: 'lines',
    line: {color: '#80CAF6'}
  }];
  var gelement = this.$('#graph')[0];
  Plotly.plot(gelement,ndata);
  
  var cnt = 0;
  var interval = setInterval(function() {

    var time = new Date();
    var olderTime = time.setMinutes(time.getMinutes() - 2);
    var futureTime = time.setMinutes(time.getMinutes() + 2);

    var minuteView = {
      xaxis: {
        type: 'date',
        range: [olderTime,futureTime]
      }
    };

    Plotly.relayout('graph', minuteView);

    if(cnt === 100) clearInterval(interval);
  }, 1000);
  
  const handle = data.observeChanges ({
    added(id, wave) {
      var xval = wave.times;
      var yval = wave.data;
      
      xval = xval.map( function(element) {
        var temp = new Date(parseInt(element));
        return temp;
      });
      
      var update = {
        x:  [xval],
        y:  [yval]
      };
      Plotly.extendTraces(gelement, update, [0]);
    }
  });
  
});

