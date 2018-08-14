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
Session.set("ChanN",undefined);
Session.set("ChanE",undefined);
Session.set("ChanZ",undefined);
  
Template.info.onCreated(function () {
  Session.set("Sta",undefined);
  Session.set("ChanN",undefined);
  Session.set("ChanE",undefined);
  Session.set("ChanZ",undefined);
});

Template.info.events({
  'submit .form-station' (event, instance) {
    event.preventDefault();
    var getStation = event.target.station.value;
    var getChannel = event.target.channel.value;
    
    var getChanN = getChannel + 'N';
    var getChanE = getChannel + 'E';
    var getChanZ = getChannel + 'Z';
    
    console.log(getChannel);
    console.log(getChanN);
    console.log(getChanE);
    console.log(getChanZ);
    
    Meteor.call('checkcol', { 
        sta: getStation, 
        chan: getChanN
      }, (err, res) => {
        if (err){
          alert(err);
        } else {
          Session.set("Sta", getStation);
          Session.set("ChanN", getChanN);
          //Success on Chan N!
        }
    });
    
    Meteor.call('checkcol', { 
        sta: getStation, 
        chan: getChanE
      }, (err, res) => {
        if (err){
          alert(err);
        } else {
          Session.set("Sta", getStation);
          Session.set("ChanE", getChanE);
          //Success on Chan E!
        }
    });
    
    Meteor.call('checkcol', { 
        sta: getStation, 
        chan: getChanZ
      }, (err, res) => {
        if (err){
          alert(err);
        } else {
          Session.set("Sta", getStation);
          Session.set("ChanZ", getChanZ);
          Router.go('/graph');
          //Success on Chan Z!
        }
    });
    
  }
});

Template.graph.helpers({
  foo() {
    return Session.get("Sta");
  },
  dooN() {
    return Session.get("ChanN");
  },
  dooE() {
    return Session.get("ChanE");
  },
  dooZ() {
    return Session.get("ChanZ");
  }
});

Template.graph.onRendered(function () {
  // Check if station and channel are undefined
  var sta = Session.get("Sta");
  var chaN = Session.get("ChanN");
  var chaE = Session.get("ChanE");
  var chaZ = Session.get("ChanZ");
  if (sta === undefined || chaZ === undefined){
    Router.go('/');
  }
  
  // Get Data
  var dataN = get_collection(sta).find({'channel': chaN});
  var dataE = get_collection(sta).find({'channel': chaE});
  var dataZ = get_collection(sta).find({'channel': chaZ});
  var time = [];
  var traceN = [];
  var traceE = [];
  var traceZ = [];
  
  /*data.forEach( function(element) {
    time = time.concat(element.times);
    trace = trace.concat(element.data);
  }); 
  time = time.map( function (element) {
    return new Date(element);
  });*/
  
  var ndata = [{
    x: time, 
    y: traceN,
    mode: 'lines',
    line: {color: '#80CAF6'}
  }];
  
  var edata = [{
    x: time, 
    y: traceE,
    mode: 'lines',
    line: {color: '#80CAF6'}
  }];
  
  var zdata = [{
    x: time, 
    y: traceZ,
    mode: 'lines',
    line: {color: '#80CAF6'}
  }];
  
  var layoutN = {
    title: 'Station ' + sta + ', Channel ' + chaN,
    yaxis: {
      title: 'Counts',
      titlefont: {
        family: 'Courier New, monospace',
        size: 18,
        color: '#7f7f7f'
      }
    },
  };
  
  var layoutE = {
    title: 'Station ' + sta + ', Channel ' + chaE,
    yaxis: {
      title: 'Counts',
      titlefont: {
        family: 'Courier New, monospace',
        size: 18,
        color: '#7f7f7f'
      }
    },
  };
  
  var layoutZ = {
    title: 'Station ' + sta + ', Channel ' + chaZ,
    yaxis: {
      title: 'Counts',
      titlefont: {
        family: 'Courier New, monospace',
        size: 18,
        color: '#7f7f7f'
      }
    },
  };
  
  // Plot
  var gelementN = this.$('#graphN')[0];
  Plotly.plot(gelementN, ndata, layoutN);
  
  var gelementE = this.$('#graphE')[0];
  Plotly.plot(gelementE, edata, layoutE);
  
  var gelementZ = this.$('#graphZ')[0];
  Plotly.plot(gelementZ, zdata, layoutZ);
  
  // Roll changes
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

    Plotly.relayout('graphN', minuteView);
    Plotly.relayout('graphE', minuteView);
    Plotly.relayout('graphZ', minuteView);

    if(cnt === 100) clearInterval(interval);
  }, 1000);
  
  // Handle new data!!
  const handleN = dataN.observeChanges ({
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
      Plotly.extendTraces(gelementN, update, [0]);
    }
  });
  
  const handleE = dataE.observeChanges ({
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
      Plotly.extendTraces(gelementE, update, [0]);
    }
  });
  
  const handleZ = dataZ.observeChanges ({
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
      Plotly.extendTraces(gelementZ, update, [0]);
    }
  });
  
});

