from sklearn import tree
import graphviz
import json

print "Reading data..."
ovc = None
with open("order.json") as f:
	ovc = json.load( f )

print "Fitting data..."
dt = tree.DecisionTreeClassifier()
dt = dt.fit( ovc['data'], ovc['target'] )

print "Score:", dt.score( ovc['data'], ovc['target'] )

print "Exporting..."
tree.export_graphviz( dt, out_file="tree.dot" ) 
