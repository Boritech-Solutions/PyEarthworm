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

import { Meteor } from 'meteor/meteor';
import { Mongo } from 'meteor/mongo';
import { get_collection } from '../lib/collection_manager.js'
import '../lib/routes.js';

Meteor.startup(() => {
  //var TestVar = Mongo.Collection.getAll();
});

Meteor.methods({
  'checkcol'({sta,chan}){
    //console.log(sta);
    //console.log(chan);
    //console.log(get_collection(sta).find({'channel': chan}).count());
    if( get_collection(sta).find({'channel': chan}).count() !== 0 ) {
      //console.log("Collection not empty");
      return;
    }
    else{
      // Remove from the collection manager
      //console.log("Collection empty");
      throw new Meteor.Error(5, 'Station/Channel not found', 'Collection empty');
    }
  }
});
