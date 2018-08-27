import './graph.html';
import { FlowRouter } from 'meteor/kadira:flow-router';
import { getCollection } from '/imports/api/collections_manager/collections_manager.js';
import { Meteor } from 'meteor/meteor';

//var d3 = require("d3");
var c3 = require("c3");

Template.graph.onCreated(function graphOnCreated() {
  staname = FlowRouter.getParam('_id');
  Meteor.call('checkcol', {
    sta: staname }, 
    (err, res) => {
      if (err){
        alert(err);
        FlowRouter.go('App.home',{});
      } 
    }
  );
  console.log(staname);
  Meteor.subscribe(staname);
  mystation = getCollection(staname);
  mychans = mystation.find({});
});

Template.graph.helpers({
  counter() {
    return mystation.find({}).count();
  },
  station() {
    return staname;
  },
  channels (){
    return mystation.find({});
  }
});

Template.graph.events({
  'click button'(event, instance) {

    console.log(mychans.count());

    mychans.forEach( (element) => {
      //console.log(element['scnl']);

      // Get element
      var gelement = document.getElementById(element['scnl']);
      
      // Convert time from ms to ISO STRING
      element['time'] = element['time'].map( function(element) {
        var temp = new Date(parseInt(element)).toISOString();
        return temp;
      });

      // Prepare the data array
      var time = element['time'].slice();
      var data = element['wave'].slice();
      time.unshift('time');
      data.unshift(element['scnl']);

      // Generate chart
      var chart = c3.generate({
        bindto: gelement,
        data: {
          x: 'time',
          xFormat: '%Y-%m-%dT%H:%M:%S.%LZ',
          columns: [
            time,
            data
          ]
        },
        point: {
          show: false
        },
        axis: {
          x: {
            type: 'timeseries',
            tick: {
              format: '%H:%M:%S.%L'
            }
          }
        }
      });
      //console.log(gelement);
    });
  },
});

Template.graph.onRendered(function () {

});